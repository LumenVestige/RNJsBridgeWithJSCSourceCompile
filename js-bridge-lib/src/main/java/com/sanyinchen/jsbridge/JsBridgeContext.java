/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * <p>
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;

import androidx.annotation.Nullable;

import com.facebook.infer.annotation.Assertions;
import com.sanyinchen.jsbridge.base.JsBridgeInstance;
import com.sanyinchen.jsbridge.context.JavaScriptContextHolder;
import com.sanyinchen.jsbridge.exception.NativeModuleCallExceptionHandler;
import com.sanyinchen.jsbridge.lifecycle.ActivityEventListener;
import com.sanyinchen.jsbridge.lifecycle.LifecycleEventListener;
import com.sanyinchen.jsbridge.lifecycle.LifecycleState;
import com.sanyinchen.jsbridge.module.bridge.NativeModule;
import com.sanyinchen.jsbridge.module.js.JavaScriptModule;
import com.sanyinchen.jsbridge.queue.MessageQueueThread;
import com.sanyinchen.jsbridge.queue.ReactQueueConfiguration;
import com.sanyinchen.jsbridge.utils.UiThreadUtil;

import java.lang.ref.WeakReference;
import java.util.concurrent.CopyOnWriteArraySet;


/**
 * Abstract ContextWrapper for Android application or activity {@link Context} and
 * {@link JsBridgeInstance}
 */
public class JsBridgeContext extends ContextWrapper {

    private static final String EARLY_JS_ACCESS_EXCEPTION_MESSAGE =
            "Tried to access a JS module before the React instance was fully set up. Calls to " +
                    "ReactContext#getJSModule should only happen once initialize() has been called on your " +
                    "native module.";

    private final CopyOnWriteArraySet<LifecycleEventListener> mLifecycleEventListeners =
            new CopyOnWriteArraySet<>();
    private final CopyOnWriteArraySet<ActivityEventListener> mActivityEventListeners =
            new CopyOnWriteArraySet<>();


    private @Nullable
    JsBridgeInstance mCatalystInstance;
    private @Nullable
    MessageQueueThread mNativeModulesMessageQueueThread;
    private @Nullable
    MessageQueueThread mJSMessageQueueThread;
    private @Nullable
    NativeModuleCallExceptionHandler mNativeModuleCallExceptionHandler;
    private @Nullable
    WeakReference<Activity> mCurrentActivity;
    private LifecycleState mLifecycleState = LifecycleState.BEFORE_CREATE;

    public JsBridgeContext(Context base) {
        super(base);
    }

    /**
     * Set and initialize CatalystInstance for this Context. This should be called exactly once.
     */
    public void initializeWithInstance(JsBridgeInstance catalystInstance) {
        if (catalystInstance == null) {
            throw new IllegalArgumentException("CatalystInstance cannot be null.");
        }
        if (mCatalystInstance != null) {
            throw new IllegalStateException("ReactContext has been already initialized");
        }

        mCatalystInstance = catalystInstance;

        ReactQueueConfiguration queueConfig = catalystInstance.getReactQueueConfiguration();
        mNativeModulesMessageQueueThread = queueConfig.getNativeModulesQueueThread();
        mJSMessageQueueThread = queueConfig.getJSQueueThread();
    }

    public void resetPerfStats() {
        if (mNativeModulesMessageQueueThread != null) {
            mNativeModulesMessageQueueThread.resetPerfStats();
        }
        if (mJSMessageQueueThread != null) {
            mJSMessageQueueThread.resetPerfStats();
        }
    }

    public void setNativeModuleCallExceptionHandler(
            @Nullable NativeModuleCallExceptionHandler nativeModuleCallExceptionHandler) {
        mNativeModuleCallExceptionHandler = nativeModuleCallExceptionHandler;
    }

    // We override the following method so that views inflated with the inflater obtained from this
    // context return the ReactContext in #getContext(). The default implementation uses the base
    // context instead, so it couldn't be cast to ReactContext.
    // TODO: T7538796 Check requirement for Override of getSystemService ReactContext
    @Override
    public Object getSystemService(String name) {
        return getBaseContext().getSystemService(name);
    }

    /**
     * @return handle to the specified JS module for the CatalystInstance associated with this Context
     */
    public <T extends JavaScriptModule> T getJSModule(Class<T> jsInterface) {
        if (mCatalystInstance == null) {
            throw new RuntimeException(EARLY_JS_ACCESS_EXCEPTION_MESSAGE);
        }
        return mCatalystInstance.getJSModule(jsInterface);
    }

    public LifecycleState getLifecycleState() {
        return mLifecycleState;
    }

    public <T extends NativeModule> boolean hasNativeModule(Class<T> nativeModuleInterface) {
        if (mCatalystInstance == null) {
            throw new RuntimeException(
                    "Trying to call native module before CatalystInstance has been set!");
        }
        return mCatalystInstance.hasNativeModule(nativeModuleInterface);
    }

    /**
     * @return the instance of the specified module interface associated with this ReactContext.
     */
    public <T extends NativeModule> T getNativeModule(Class<T> nativeModuleInterface) {
        if (mCatalystInstance == null) {
            throw new RuntimeException(
                    "Trying to call native module before CatalystInstance has been set!");
        }
        return mCatalystInstance.getNativeModule(nativeModuleInterface);
    }

    public JsBridgeInstance getCatalystInstance() {
        return Assertions.assertNotNull(mCatalystInstance);
    }

    public boolean hasActiveCatalystInstance() {
        return mCatalystInstance != null && !mCatalystInstance.isDestroyed();
    }


    public void removeLifecycleEventListener(LifecycleEventListener listener) {
        mLifecycleEventListeners.remove(listener);
    }

    public void addActivityEventListener(ActivityEventListener listener) {
        mActivityEventListeners.add(listener);
    }

    public void removeActivityEventListener(ActivityEventListener listener) {
        mActivityEventListeners.remove(listener);
    }

    /**
     * Should be called by the hosting Fragment in {@link Fragment#onResume}
     */
    public void onHostResume(@Nullable Activity activity) {
        mCurrentActivity = new WeakReference(activity);
        for (LifecycleEventListener listener : mLifecycleEventListeners) {
            try {
                listener.onHostResume();
            } catch (RuntimeException e) {
                handleException(e);
            }
        }
    }

    public void onNewIntent(@Nullable Activity activity, Intent intent) {
        UiThreadUtil.assertOnUiThread();
        mCurrentActivity = new WeakReference(activity);
        for (ActivityEventListener listener : mActivityEventListeners) {
            try {
                listener.onNewIntent(intent);
            } catch (RuntimeException e) {
                handleException(e);
            }
        }
    }

    /**
     * Should be called by the hosting Fragment in {@link Fragment#onPause}
     */
    public void onHostPause() {
        for (LifecycleEventListener listener : mLifecycleEventListeners) {
            try {
                listener.onHostPause();
            } catch (RuntimeException e) {
                handleException(e);
            }
        }
    }

    /**
     * Should be called by the hosting Fragment in {@link Fragment#onDestroy}
     */
    public void onHostDestroy() {
        UiThreadUtil.assertOnUiThread();
        for (LifecycleEventListener listener : mLifecycleEventListeners) {
            try {
                listener.onHostDestroy();
            } catch (RuntimeException e) {
                handleException(e);
            }
        }
        mCurrentActivity = null;
    }

    /**
     * Destroy this instance, making it unusable.
     */
    public void destroy() {
        UiThreadUtil.assertOnUiThread();

        if (mCatalystInstance != null) {
            mCatalystInstance.destroy();
        }
    }

    /**
     * Should be called by the hosting Fragment in {@link Fragment#onActivityResult}
     */
    public void onActivityResult(Activity activity, int requestCode, int resultCode, Intent data) {
        for (ActivityEventListener listener : mActivityEventListeners) {
            try {
                listener.onActivityResult(activity, requestCode, resultCode, data);
            } catch (RuntimeException e) {
                handleException(e);
            }
        }
    }

    public void assertOnNativeModulesQueueThread() {
        Assertions.assertNotNull(mNativeModulesMessageQueueThread).assertIsOnThread();
    }

    public void assertOnNativeModulesQueueThread(String message) {
        Assertions.assertNotNull(mNativeModulesMessageQueueThread).assertIsOnThread(message);
    }

    public boolean isOnNativeModulesQueueThread() {
        return Assertions.assertNotNull(mNativeModulesMessageQueueThread).isOnThread();
    }

    public void runOnNativeModulesQueueThread(Runnable runnable) {
        Assertions.assertNotNull(mNativeModulesMessageQueueThread).runOnQueue(runnable);
    }

    public void assertOnJSQueueThread() {
        Assertions.assertNotNull(mJSMessageQueueThread).assertIsOnThread();
    }

    public boolean isOnJSQueueThread() {
        return Assertions.assertNotNull(mJSMessageQueueThread).isOnThread();
    }

    public void runOnJSQueueThread(Runnable runnable) {
        Assertions.assertNotNull(mJSMessageQueueThread).runOnQueue(runnable);
    }

    /**
     * Passes the given exception to the current
     * {@link com.facebook.react.bridge.NativeModuleCallExceptionHandler} if one exists, rethrowing
     * otherwise.
     */
    public void handleException(Exception e) {
        if (mCatalystInstance != null &&
                !mCatalystInstance.isDestroyed() &&
                mNativeModuleCallExceptionHandler != null) {
            mNativeModuleCallExceptionHandler.handleException(e);
        } else {
            throw new RuntimeException(e);
        }
    }

    public boolean hasCurrentActivity() {
        return mCurrentActivity != null && mCurrentActivity.get() != null;
    }

    /**
     * Same as {@link Activity#startActivityForResult(Intent, int)}, this just redirects the call to
     * the current activity. Returns whether the activity was started, as this might fail if this
     * was called before the context is in the right state.
     */
    public boolean startActivityForResult(Intent intent, int code, Bundle bundle) {
        Activity activity = getCurrentActivity();
        Assertions.assertNotNull(activity);
        activity.startActivityForResult(intent, code, bundle);
        return true;
    }

    /**
     * Get the activity to which this context is currently attached, or {@code null} if not attached.
     * DO NOT HOLD LONG-LIVED REFERENCES TO THE OBJECT RETURNED BY THIS METHOD, AS THIS WILL CAUSE
     * MEMORY LEAKS.
     */
    public @Nullable
    Activity getCurrentActivity() {
        if (mCurrentActivity == null) {
            return null;
        }
        return mCurrentActivity.get();
    }

    /**
     * Get the C pointer (as a long) to the JavaScriptCore context associated with this instance. Use
     * the following pattern to ensure that the JS context is not cleared while you are using it:
     * JavaScriptContextHolder jsContext = reactContext.getJavaScriptContextHolder()
     * synchronized(jsContext) { nativeThingNeedingJsContext(jsContext.get()); }
     */
    public JavaScriptContextHolder getJavaScriptContextHolder() {
        return mCatalystInstance.getJavaScriptContextHolder();
    }

}
