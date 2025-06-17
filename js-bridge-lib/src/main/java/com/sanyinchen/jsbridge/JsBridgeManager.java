/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * <p>
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge;


import android.app.Activity;
import android.content.Context;
import android.os.Process;


import androidx.annotation.Nullable;

import com.facebook.infer.annotation.Assertions;
import com.facebook.infer.annotation.ThreadConfined;
import com.facebook.infer.annotation.ThreadSafe;
import com.facebook.soloader.SoLoader;
import com.sanyinchen.jsbridge.base.JsBridgeInstance;
import com.sanyinchen.jsbridge.exception.NativeModuleCallExceptionHandler;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutor;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutorFactory;
import com.sanyinchen.jsbridge.load.JSBundleLoader;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleRegistry;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleRegistryBuilder;
import com.sanyinchen.jsbridge.module.bridge.NativeModelPackage;
import com.sanyinchen.jsbridge.module.jsi.JSIModulePackage;
import com.sanyinchen.jsbridge.queue.ReactQueueConfigurationSpec;
import com.sanyinchen.jsbridge.utils.UiThreadUtil;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import static com.facebook.infer.annotation.ThreadConfined.UI;


@ThreadSafe
public class JsBridgeManager {
    static {
        SoLoader.loadLibrary("js-bridge");
    }

    private @Nullable
    @ThreadConfined(UI)
    ReactContextInitParams mPendingReactContextInitParams;
    private volatile @Nullable
    Thread mCreateReactContextThread;
    /* accessed from any thread */
    private final JavaScriptExecutorFactory mJavaScriptExecutorFactory;

    private final @Nullable
    JSBundleLoader mBundleLoader;
    private final @Nullable
    String mJSMainModulePath; /* path to JS bundle root on packager server */
    private final List<NativeModelPackage> mPackages;
    private final Object mReactContextLock = new Object();
    private @Nullable
    volatile JsBridgeContext mCurrentReactContext;
    private final Context mApplicationContext;
    private @Nullable
    Activity mCurrentActivity;
    private final Collection<ReactInstanceEventListener> mReactInstanceEventListeners =
            Collections.synchronizedSet(new HashSet<ReactInstanceEventListener>());
    // Identifies whether the instance manager is or soon will be initialized (on background thread)
    private volatile boolean mHasStartedCreatingInitialContext = false;
    // Identifies whether the instance manager destroy function is in process,
    // while true any spawned create thread should wait for proper clean up before initializing
    private volatile Boolean mHasStartedDestroying = false;
    private final @Nullable
    NativeModuleCallExceptionHandler mNativeModuleCallExceptionHandler;
    private final @Nullable
    JSIModulePackage mJSIModulePackage;

    /**
     * Listener interface for react instance events.
     */
    public interface ReactInstanceEventListener {

        /**
         * Called when the react context is initialized (all modules registered). Always called on the
         * UI thread.
         */
        void onReactContextInitialized(JsBridgeContext context);
    }

    private class ReactContextInitParams {
        private final JavaScriptExecutorFactory mJsExecutorFactory;
        private final JSBundleLoader mJsBundleLoader;

        public ReactContextInitParams(
                JavaScriptExecutorFactory jsExecutorFactory,
                JSBundleLoader jsBundleLoader) {
            mJsExecutorFactory = Assertions.assertNotNull(jsExecutorFactory);
            mJsBundleLoader = Assertions.assertNotNull(jsBundleLoader);
        }

        public JavaScriptExecutorFactory getJsExecutorFactory() {
            return mJsExecutorFactory;
        }

        public JSBundleLoader getJsBundleLoader() {
            return mJsBundleLoader;
        }
    }


    /* package */
    JsBridgeManager(
            Context applicationContext,
            @Nullable Activity currentActivity,
            JavaScriptExecutorFactory javaScriptExecutorFactory,
            @Nullable JSBundleLoader bundleLoader,
            @Nullable String jsMainModulePath,
            List<NativeModelPackage> packages,
            NativeModuleCallExceptionHandler nativeModuleCallExceptionHandler,
            @Nullable JSIModulePackage jsiModulePackage) {
        initializeSoLoaderIfNecessary(applicationContext);

        mApplicationContext = applicationContext;
        mCurrentActivity = currentActivity;
        mJavaScriptExecutorFactory = javaScriptExecutorFactory;
        mBundleLoader = bundleLoader;
        mJSMainModulePath = jsMainModulePath;
        mPackages = new ArrayList<>();
        mNativeModuleCallExceptionHandler = nativeModuleCallExceptionHandler;
        synchronized (mPackages) {
            mPackages.addAll(packages);
        }
        mJSIModulePackage = jsiModulePackage;
    }

    private static void initializeSoLoaderIfNecessary(Context applicationContext) {
        // Call SoLoader.initialize here, this is required for apps that does not use exopackage and
        // does not use SoLoader for loading other native code except from the one used by React Native
        // This way we don't need to require others to have additional initialization code and to
        // subclass android.app.Application.

        // Method SoLoader.init is idempotent, so if you wish to use native exopackage, just call
        // SoLoader.init with appropriate args before initializing ReactInstanceManager
        SoLoader.init(applicationContext, /* native exopackage */ false);
    }

    @ThreadConfined(UI)
    public void run() {
        Assertions.assertCondition(
                !mHasStartedCreatingInitialContext,
                "createReactContextInBackground should only be called when creating the react " +
                        "application for the first time. When reloading JS, e.g. from a new file, explicitly" +
                        "use recreateReactContextInBackground");

        mHasStartedCreatingInitialContext = true;
        recreateReactContextInBackgroundInner();
    }

    @Nullable
    public JsBridgeContext getCurrentReactContext() {
        return mCurrentReactContext;
    }

    @ThreadConfined(UI)
    private void recreateReactContextInBackgroundInner() {
        recreateReactContextInBackgroundFromBundleLoader();
    }

    @ThreadConfined(UI)
    private void recreateReactContextInBackgroundFromBundleLoader() {
        recreateReactContextInBackground(mJavaScriptExecutorFactory, mBundleLoader);
    }

    @ThreadConfined(UI)
    private void recreateReactContextInBackground(
            JavaScriptExecutorFactory jsExecutorFactory,
            JSBundleLoader jsBundleLoader) {
        final ReactContextInitParams initParams = new ReactContextInitParams(
                jsExecutorFactory,
                jsBundleLoader);
        if (mCreateReactContextThread == null) {
            runCreateReactContextOnNewThread(initParams);
        } else {
            mPendingReactContextInitParams = initParams;
        }
    }

    @ThreadConfined(UI)
    private void runCreateReactContextOnNewThread(final ReactContextInitParams initParams) {
        mCreateReactContextThread =
                new Thread(
                        null,
                        new Runnable() {
                            @Override
                            public void run() {
                                synchronized (JsBridgeManager.this.mHasStartedDestroying) {
                                    while (JsBridgeManager.this.mHasStartedDestroying) {
                                        try {
                                            JsBridgeManager.this.mHasStartedDestroying.wait();
                                        } catch (InterruptedException e) {
                                            continue;
                                        }
                                    }
                                }
                                // As destroy() may have run and set this to false, ensure that it is true before we create
                                mHasStartedCreatingInitialContext = true;

                                try {
                                    Process.setThreadPriority(Process.THREAD_PRIORITY_DISPLAY);
                                    final JsBridgeContext reactApplicationContext =
                                            createReactContext(
                                                    initParams.getJsExecutorFactory().create(),
                                                    initParams.getJsBundleLoader());

                                    mCreateReactContextThread = null;
                                    final Runnable maybeRecreateReactContextRunnable =
                                            new Runnable() {
                                                @Override
                                                public void run() {
                                                    if (mPendingReactContextInitParams != null) {
                                                        runCreateReactContextOnNewThread(mPendingReactContextInitParams);
                                                        mPendingReactContextInitParams = null;
                                                    }
                                                }
                                            };
                                    Runnable setupReactContextRunnable =
                                            new Runnable() {
                                                @Override
                                                public void run() {
                                                    try {
                                                        setupReactContext(reactApplicationContext);
                                                    } catch (Exception e) {
                                                        e.printStackTrace();
                                                        // mDevSupportManager.handleException(e);
                                                    }
                                                }
                                            };

                                    reactApplicationContext.runOnNativeModulesQueueThread(setupReactContextRunnable);
                                    UiThreadUtil.runOnUiThread(maybeRecreateReactContextRunnable);
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                            }
                        }, "create_react_context");
        mCreateReactContextThread.start();
    }

    private void setupReactContext(final JsBridgeContext reactContext) {
        synchronized (mReactContextLock) {
            mCurrentReactContext = Assertions.assertNotNull(reactContext);
        }

        JsBridgeInstance catalystInstance =
                Assertions.assertNotNull(reactContext.getCatalystInstance());

        catalystInstance.initialize();


        ReactInstanceEventListener[] listeners =
                new ReactInstanceEventListener[mReactInstanceEventListeners.size()];
        final ReactInstanceEventListener[] finalListeners =
                mReactInstanceEventListeners.toArray(listeners);

        UiThreadUtil.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        for (ReactInstanceEventListener listener : finalListeners) {
                            listener.onReactContextInitialized(reactContext);
                        }
                    }
                });
        reactContext.runOnJSQueueThread(
                new Runnable() {
                    @Override
                    public void run() {
                        Process.setThreadPriority(Process.THREAD_PRIORITY_DEFAULT);
                    }
                });
        reactContext.runOnNativeModulesQueueThread(
                new Runnable() {
                    @Override
                    public void run() {
                        Process.setThreadPriority(Process.THREAD_PRIORITY_DEFAULT);
                    }
                });
    }

    /**
     * @return instance of {@link JsBridgeContext} configured a {@link JsBridgeInstance} set
     */
    private JsBridgeContext createReactContext(
            JavaScriptExecutor jsExecutor,
            JSBundleLoader jsBundleLoader) {
        final JsBridgeContext reactContext = new JsBridgeContext(mApplicationContext);

        NativeModuleCallExceptionHandler exceptionHandler = mNativeModuleCallExceptionHandler;

        reactContext.setNativeModuleCallExceptionHandler(exceptionHandler);

        NativeModuleRegistry nativeModuleRegistry = processPackages(reactContext, mPackages, false);

        JsBridgeInstanceImpl.Builder catalystInstanceBuilder = new JsBridgeInstanceImpl.Builder()
                .setReactQueueConfigurationSpec(ReactQueueConfigurationSpec.createDefault())
                .setJSExecutor(jsExecutor)
                .setRegistry(nativeModuleRegistry)
                .setJSBundleLoader(jsBundleLoader)
                .setNativeModuleCallExceptionHandler(exceptionHandler);

        final JsBridgeInstance catalystInstance = catalystInstanceBuilder.build();

        if (mJSIModulePackage != null) {
            catalystInstance.addJSIModules(mJSIModulePackage
                    .getJSIModules(reactContext, catalystInstance.getJavaScriptContextHolder()));
        }

        catalystInstance.runJSBundle();

        reactContext.initializeWithInstance(catalystInstance);


        return reactContext;
    }

    private NativeModuleRegistry processPackages(
            JsBridgeContext reactContext,
            List<NativeModelPackage> packages,
            boolean checkAndUpdatePackageMembership) {
        NativeModuleRegistryBuilder nativeModuleRegistryBuilder = new NativeModuleRegistryBuilder(reactContext, this);

        // TODO(6818138): Solve use-case of native modules overriding
        synchronized (mPackages) {
            for (NativeModelPackage reactPackage : packages) {
                if (checkAndUpdatePackageMembership && mPackages.contains(reactPackage)) {
                    continue;
                }
                if (checkAndUpdatePackageMembership) {
                    mPackages.add(reactPackage);
                }
                processPackage(reactPackage, nativeModuleRegistryBuilder);
            }
        }
        return nativeModuleRegistryBuilder.build();
    }

    private void processPackage(
            NativeModelPackage reactPackage,
            NativeModuleRegistryBuilder nativeModuleRegistryBuilder) {
        nativeModuleRegistryBuilder.processPackage(reactPackage);
    }
}
