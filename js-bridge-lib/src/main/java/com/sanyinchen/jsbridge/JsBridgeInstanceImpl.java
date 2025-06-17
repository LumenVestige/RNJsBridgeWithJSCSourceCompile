/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge;

import android.content.res.AssetManager;
import android.os.AsyncTask;
import android.os.SystemClock;
import android.util.Log;

import androidx.annotation.Nullable;

import com.facebook.infer.annotation.Assertions;
import com.facebook.jni.HybridData;
import com.facebook.jni.annotations.DoNotStrip;
import com.facebook.soloader.SoLoader;
import com.sanyinchen.jsbridge.annotation.ReactModule;
import com.sanyinchen.jsbridge.base.JsBridgeInstance;
import com.sanyinchen.jsbridge.config.ReactConstants;
import com.sanyinchen.jsbridge.context.JavaScriptContextHolder;
import com.sanyinchen.jsbridge.common.callback.JsBridgeCallback;
import com.sanyinchen.jsbridge.data.NativeArray;
import com.sanyinchen.jsbridge.data.NativeArrayInterface;
import com.sanyinchen.jsbridge.data.WritableNativeArray;
import com.sanyinchen.jsbridge.exception.NativeModuleCallExceptionHandler;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutor;
import com.sanyinchen.jsbridge.load.JSBundleLoader;
import com.sanyinchen.jsbridge.module.bridge.NativeModule;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleHolder;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleRegistry;
import com.sanyinchen.jsbridge.module.impl.java.JavaModuleWrapper;
import com.sanyinchen.jsbridge.module.js.JavaScriptModule;
import com.sanyinchen.jsbridge.module.js.JavaScriptModuleRegistry;
import com.sanyinchen.jsbridge.module.jsi.JSIModule;
import com.sanyinchen.jsbridge.module.jsi.JSIModuleRegistry;
import com.sanyinchen.jsbridge.module.jsi.JSIModuleSpec;
import com.sanyinchen.jsbridge.queue.MessageQueueThread;
import com.sanyinchen.jsbridge.queue.QueueThreadExceptionHandler;
import com.sanyinchen.jsbridge.queue.ReactQueueConfiguration;
import com.sanyinchen.jsbridge.queue.ReactQueueConfigurationImpl;
import com.sanyinchen.jsbridge.queue.ReactQueueConfigurationSpec;
import com.sanyinchen.jsbridge.utils.UiThreadUtil;
import com.sanyinchen.jsbridge.utils.log.FLog;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;


/**
 * This provides an implementation of the public CatalystInstance instance.  It is public because
 * it is built by XReactInstanceManager which is in a different package.
 */

@DoNotStrip
public class JsBridgeInstanceImpl implements JsBridgeInstance {
    static {
        SoLoader.loadLibrary("js-bridge");
    }

    private static final AtomicInteger sNextInstanceIdForTrace = new AtomicInteger(1);

    public static class PendingJSCall {

        public String mModule;
        public String mMethod;
        public @Nullable
        NativeArray mArguments;

        public PendingJSCall(String module, String method, @Nullable NativeArray arguments) {
            mModule = module;
            mMethod = method;
            mArguments = arguments;
        }

        void call(JsBridgeInstanceImpl catalystInstance) {
            NativeArray arguments = mArguments != null ? mArguments : new WritableNativeArray();
            catalystInstance.jniCallJSFunction(mModule, mMethod, arguments);
        }

        public String toString() {
            return mModule + "." + mMethod + "("
                    + (mArguments == null ? "" : mArguments.toString()) + ")";
        }
    }

    // Access from any thread
    private final ReactQueueConfigurationImpl mReactQueueConfiguration;
    private final AtomicInteger mPendingJSCalls = new AtomicInteger(0);
    private final String mJsPendingCallsTitleForTrace =
            "pending_js_calls_instance" + sNextInstanceIdForTrace.getAndIncrement();
    private volatile boolean mDestroyed = false;
    private final JavaScriptModuleRegistry mJSModuleRegistry;
    private final JSBundleLoader mJSBundleLoader;
    private final ArrayList<PendingJSCall> mJSCallsPendingInit = new ArrayList<PendingJSCall>();
    private final Object mJSCallsPendingInitLock = new Object();

    private final NativeModuleRegistry mNativeModuleRegistry;
    private final JSIModuleRegistry mJSIModuleRegistry = new JSIModuleRegistry();
    private final NativeModuleCallExceptionHandler mNativeModuleCallExceptionHandler;
    private final MessageQueueThread mNativeModulesQueueThread;
    private boolean mInitialized = false;
    private volatile boolean mAcceptCalls = false;

    private boolean mJSBundleHasLoaded;
    private @Nullable
    String mSourceURL;

    private JavaScriptContextHolder mJavaScriptContextHolder;

    // C++ parts
    private final HybridData mHybridData;

    private native static HybridData initHybrid();

    private JsBridgeInstanceImpl(
            final ReactQueueConfigurationSpec reactQueueConfigurationSpec,
            final JavaScriptExecutor jsExecutor,
            final NativeModuleRegistry nativeModuleRegistry,
            final JSBundleLoader jsBundleLoader,
            NativeModuleCallExceptionHandler nativeModuleCallExceptionHandler) {
        mHybridData = initHybrid();

        mReactQueueConfiguration = ReactQueueConfigurationImpl.create(
                reactQueueConfigurationSpec,
                new NativeExceptionHandler());
        mNativeModuleRegistry = nativeModuleRegistry;
        mJSModuleRegistry = new JavaScriptModuleRegistry();
        mJSBundleLoader = jsBundleLoader;
        mNativeModuleCallExceptionHandler = nativeModuleCallExceptionHandler;
        mNativeModulesQueueThread = mReactQueueConfiguration.getNativeModulesQueueThread();
        initializeBridge(
                new BridgeCallback(this),
                jsExecutor,
                mReactQueueConfiguration.getJSQueueThread(),
                mNativeModulesQueueThread,
                mNativeModuleRegistry.getJavaModules(this),
                mNativeModuleRegistry.getCxxModules());
        mJavaScriptContextHolder = new JavaScriptContextHolder(getJavaScriptContext());
    }

    private static class BridgeCallback implements JsBridgeCallback {
        // We do this so the callback doesn't keep the CatalystInstanceImpl alive.
        // In this case, the callback is held in C++ code, so the GC can't see it
        // and determine there's an inaccessible cycle.
        private final WeakReference<JsBridgeInstanceImpl> mOuter;

        BridgeCallback(JsBridgeInstanceImpl outer) {
            mOuter = new WeakReference<>(outer);
        }

        @Override
        public void onBatchComplete() {
            JsBridgeInstanceImpl impl = mOuter.get();
            if (impl != null) {
                impl.mNativeModuleRegistry.onBatchComplete();
            }
        }

        @Override
        public void incrementPendingJSCalls() {
            JsBridgeInstanceImpl impl = mOuter.get();
            if (impl != null) {
                impl.incrementPendingJSCalls();
            }
        }

        @Override
        public void decrementPendingJSCalls() {
            JsBridgeInstanceImpl impl = mOuter.get();
            if (impl != null) {
                impl.decrementPendingJSCalls();
            }
        }
    }

    /**
     * This method and the native below permits a CatalystInstance to extend the known
     * Native modules. This registry contains only the new modules to load. The
     * registry {@code mNativeModuleRegistry} updates internally to contain all the new modules, and generates
     * the new registry for extracting just the new collections.
     */
    @Override
    public void extendNativeModules(NativeModuleRegistry modules) {
        //Extend the Java-visible registry of modules
        mNativeModuleRegistry.registerModules(modules);
        Collection<JavaModuleWrapper> javaModules = modules.getJavaModules(this);
        Collection<NativeModuleHolder> cxxModules = modules.getCxxModules();
        //Extend the Cxx-visible registry of modules wrapped in appropriate interfaces
        jniExtendNativeModules(javaModules, cxxModules);
    }

    private native void jniExtendNativeModules(
            Collection<JavaModuleWrapper> javaModules,
            Collection<NativeModuleHolder> cxxModules);

    private native void initializeBridge(
            JsBridgeCallback callback,
            JavaScriptExecutor jsExecutor,
            MessageQueueThread jsQueue,
            MessageQueueThread moduleQueue,
            Collection<JavaModuleWrapper> javaModules,
            Collection<NativeModuleHolder> cxxModules);


    @Override
    public void loadScriptFromAssets(AssetManager assetManager, String assetURL, boolean loadSynchronously) {
        mSourceURL = assetURL;
        jniLoadScriptFromAssets(assetManager, assetURL, loadSynchronously);
    }

    private native void jniLoadScriptFromAssets(AssetManager assetManager, String assetURL, boolean loadSynchronously);


    @Override
    public void runJSBundle() {
        Assertions.assertCondition(!mJSBundleHasLoaded, "JS bundle was already loaded!");
        // incrementPendingJSCalls();
        mJSBundleLoader.loadScript(JsBridgeInstanceImpl.this);

        synchronized (mJSCallsPendingInitLock) {

            // Loading the bundle is queued on the JS thread, but may not have
            // run yet.  It's safe to set this here, though, since any work it
            // gates will be queued on the JS thread behind the load.
            mAcceptCalls = true;

            for (PendingJSCall function : mJSCallsPendingInit) {
                function.call(this);
            }
            mJSCallsPendingInit.clear();
            mJSBundleHasLoaded = true;
        }
    }

    @Override
    public boolean hasRunJSBundle() {
        synchronized (mJSCallsPendingInitLock) {
            return mJSBundleHasLoaded && mAcceptCalls;
        }
    }

    @Override
    public @Nullable
    String getSourceURL() {
        return mSourceURL;
    }

    private native void jniCallJSFunction(
            String module,
            String method,
            NativeArray arguments);

    @Override
    public void callFunction(
            final String module,
            final String method,
            final NativeArray arguments) {
        callFunction(new PendingJSCall(module, method, arguments));
    }

    public void callFunction(PendingJSCall function) {
        if (mDestroyed) {
            final String call = function.toString();
            FLog.w(ReactConstants.TAG, "Calling JS function after bridge has been destroyed: " + call);
            return;
        }
        if (!mAcceptCalls) {
            // Most of the time the instance is initialized and we don't need to acquire the lock
            synchronized (mJSCallsPendingInitLock) {
                if (!mAcceptCalls) {
                    mJSCallsPendingInit.add(function);
                    return;
                }
            }
        }
        function.call(this);
    }

    private native void jniCallJSCallback(int callbackID, NativeArray arguments);

    @Override
    public void invokeCallback(final int callbackID, final NativeArrayInterface arguments) {

        if (mDestroyed) {
            FLog.w(ReactConstants.TAG, "Invoking JS callback after bridge has been destroyed.");
            return;
        }

        jniCallJSCallback(callbackID, (NativeArray) arguments);
    }

    /**
     * Destroys this catalyst instance, waiting for any other threads in ReactQueueConfiguration
     * (besides the UI thread) to finish running. Must be called from the UI thread so that we can
     * fully shut down other threads.
     */
    @Override
    public void destroy() {
        Log.d(ReactConstants.TAG, "CatalystInstanceImpl.destroy() start");
        UiThreadUtil.assertOnUiThread();

        if (mDestroyed) {
            return;
        }

        mDestroyed = true;

        mNativeModulesQueueThread.runOnQueue(
                new Runnable() {
                    @Override
                    public void run() {
                        mNativeModuleRegistry.notifyJSInstanceDestroy();
                        mJSIModuleRegistry.notifyJSInstanceDestroy();
                        AsyncTask.execute(
                                new Runnable() {
                                    @Override
                                    public void run() {
                                        // Kill non-UI threads from neutral third party
                                        // potentially expensive, so don't run on UI thread

                                        // contextHolder is used as a lock to guard against other users of the JS VM
                                        // having
                                        // the VM destroyed underneath them, so notify them before we resetNative
                                        mJavaScriptContextHolder.clear();

                                        mHybridData.resetNative();
                                        getReactQueueConfiguration().destroy();
                                        Log.d(ReactConstants.TAG, "CatalystInstanceImpl.destroy() end");
                                    }
                                });
                    }
                });
    }

    @Override
    public boolean isDestroyed() {
        return mDestroyed;
    }

    /**
     * Initialize all the native modules
     */
    @Override
    public void initialize() {
        Log.d(ReactConstants.TAG, "CatalystInstanceImpl.initialize()");
        Assertions.assertCondition(
                !mInitialized,
                "This catalyst instance has already been initialized");
        // We assume that the instance manager blocks on running the JS bundle. If
        // that changes, then we need to set mAcceptCalls just after posting the
        // task that will run the js bundle.
        Assertions.assertCondition(
                mAcceptCalls,
                "RunJSBundle hasn't completed.");
        mInitialized = true;
        mNativeModulesQueueThread.runOnQueue(new Runnable() {
            @Override
            public void run() {
                mNativeModuleRegistry.notifyJSInstanceInitialized();
            }
        });
    }

    @Override
    public ReactQueueConfiguration getReactQueueConfiguration() {
        return mReactQueueConfiguration;
    }

    @Override
    public <T extends JavaScriptModule> T getJSModule(Class<T> jsInterface) {
        return mJSModuleRegistry.getJavaScriptModule(this, jsInterface);
    }

    @Override
    public <T extends NativeModule> boolean hasNativeModule(Class<T> nativeModuleInterface) {
        return mNativeModuleRegistry.hasModule(getNameFromAnnotation(nativeModuleInterface));
    }

    @Override
    public <T extends NativeModule> T getNativeModule(Class<T> nativeModuleInterface) {
        return (T) mNativeModuleRegistry.getModule(getNameFromAnnotation(nativeModuleInterface));
    }

    @Override
    public NativeModule getNativeModule(String moduleName) {
        return mNativeModuleRegistry.getModule(moduleName);
    }

    private <T extends NativeModule> String getNameFromAnnotation(Class<T> nativeModuleInterface) {
        ReactModule annotation = nativeModuleInterface.getAnnotation(ReactModule.class);
        if (annotation == null) {
            throw new IllegalArgumentException("Could not find @ReactModule annotation in " + nativeModuleInterface.getCanonicalName());
        }
        return annotation.name();
    }

    // This is only used by com.facebook.react.modules.common.ModuleDataCleaner
    @Override
    public Collection<NativeModule> getNativeModules() {
        return mNativeModuleRegistry.getAllModules();
    }

    private native void jniHandleMemoryPressure(int level);

    @Override
    public void handleMemoryPressure(int level) {
        if (mDestroyed) {
            return;
        }
        jniHandleMemoryPressure(level);
    }


    @Override
    public native void setGlobalVariable(String propName, String jsonValue);

    @Override
    public JavaScriptContextHolder getJavaScriptContextHolder() {
        return mJavaScriptContextHolder;
    }

    @Override
    public void addJSIModules(List<JSIModuleSpec> jsiModules) {
        mJSIModuleRegistry.registerModules(jsiModules);
    }

    @Override
    public <T extends JSIModule> T getJSIModule(Class<T> jsiModuleInterface) {
        return mJSIModuleRegistry.getModule(jsiModuleInterface);
    }

    private native long getJavaScriptContext();

    private void incrementPendingJSCalls() {
//        int oldPendingCalls = mPendingJSCalls.getAndIncrement();
//        boolean wasIdle = oldPendingCalls == 0;
//        if (wasIdle && !mBridgeIdleListeners.isEmpty()) {
//            mNativeModulesQueueThread.runOnQueue(new Runnable() {
//                @Override
//                public void run() {
//                    for (NotThreadSafeBridgeIdleDebugListener listener : mBridgeIdleListeners) {
//                        listener.onTransitionToBridgeBusy();
//                    }
//                }
//            });
//        }
    }

    private void decrementPendingJSCalls() {
//        int newPendingCalls = mPendingJSCalls.decrementAndGet();
//        // TODO(9604406): handle case of web workers injecting messages to main thread
//        //Assertions.assertCondition(newPendingCalls >= 0);
//        boolean isNowIdle = newPendingCalls == 0;
//
//        if (isNowIdle && !mBridgeIdleListeners.isEmpty()) {
//            mNativeModulesQueueThread.runOnQueue(new Runnable() {
//                @Override
//                public void run() {
//                    for (NotThreadSafeBridgeIdleDebugListener listener : mBridgeIdleListeners) {
//                        listener.onTransitionToBridgeIdle();
//                    }
//                }
//            });
//        }
    }

    private void onNativeException(Exception e) {
        mNativeModuleCallExceptionHandler.handleException(e);
        mReactQueueConfiguration.getUIQueueThread().runOnQueue(
                new Runnable() {
                    @Override
                    public void run() {
                        destroy();
                    }
                });
    }

    private class NativeExceptionHandler implements QueueThreadExceptionHandler {
        @Override
        public void handleException(Exception e) {
            // Any Exception caught here is because of something in JS. Even if it's a bug in the
            // framework/native code, it was triggered by JS and theoretically since we were able
            // to set up the bridge, JS could change its logic, reload, and not trigger that crash.
            onNativeException(e);
        }
    }


    public static class Builder {

        private @Nullable
        ReactQueueConfigurationSpec mReactQueueConfigurationSpec;
        private @Nullable
        JSBundleLoader mJSBundleLoader;
        private @Nullable
        NativeModuleRegistry mRegistry;
        private @Nullable
        JavaScriptExecutor mJSExecutor;
        private @Nullable
        NativeModuleCallExceptionHandler mNativeModuleCallExceptionHandler;


        public Builder setReactQueueConfigurationSpec(
                ReactQueueConfigurationSpec ReactQueueConfigurationSpec) {
            mReactQueueConfigurationSpec = ReactQueueConfigurationSpec;
            return this;
        }

        public Builder setRegistry(NativeModuleRegistry registry) {
            mRegistry = registry;
            return this;
        }

        public Builder setJSBundleLoader(JSBundleLoader jsBundleLoader) {
            mJSBundleLoader = jsBundleLoader;
            return this;
        }

        public Builder setJSExecutor(JavaScriptExecutor jsExecutor) {
            mJSExecutor = jsExecutor;
            return this;
        }

        public Builder setNativeModuleCallExceptionHandler(
                NativeModuleCallExceptionHandler handler) {
            mNativeModuleCallExceptionHandler = handler;
            return this;
        }

        public JsBridgeInstanceImpl build() {
            return new JsBridgeInstanceImpl(
                    Assertions.assertNotNull(mReactQueueConfigurationSpec),
                    Assertions.assertNotNull(mJSExecutor),
                    Assertions.assertNotNull(mRegistry),
                    Assertions.assertNotNull(mJSBundleLoader),
                    Assertions.assertNotNull(mNativeModuleCallExceptionHandler));
        }

    }

}
