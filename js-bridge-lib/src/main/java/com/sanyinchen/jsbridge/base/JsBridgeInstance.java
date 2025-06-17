/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * <p>
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.base;

import androidx.annotation.Nullable;

import com.facebook.jni.annotations.DoNotStrip;
import com.sanyinchen.jsbridge.context.JavaScriptContextHolder;
import com.sanyinchen.jsbridge.data.NativeArray;
import com.sanyinchen.jsbridge.data.NativeArrayInterface;
import com.sanyinchen.jsbridge.load.JSBundleLoaderDelegate;
import com.sanyinchen.jsbridge.memory.MemoryPressureListener;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleRegistry;
import com.sanyinchen.jsbridge.module.bridge.NativeModule;
import com.sanyinchen.jsbridge.module.js.JavaScriptModule;
import com.sanyinchen.jsbridge.module.jsi.JSIModule;
import com.sanyinchen.jsbridge.module.jsi.JSIModuleSpec;
import com.sanyinchen.jsbridge.queue.ReactQueueConfiguration;

import java.util.Collection;
import java.util.List;


/**
 * A higher level API on top of the asynchronous JSC bridge. This provides an
 * environment allowing the invocation of JavaScript methods and lets a set of
 * Java APIs be invokable from JavaScript as well.
 */
@DoNotStrip
public interface JsBridgeInstance
        extends MemoryPressureListener, JSInstance, JSBundleLoaderDelegate {

    void runJSBundle();

    // Returns the status of running the JS bundle; waits for an answer if runJSBundle is running
    boolean hasRunJSBundle();

    /**
     * Return the source URL of the JS Bundle that was run, or {@code null} if no JS
     * bundle has been run yet.
     */
    @Nullable
    String getSourceURL();

    // This is called from java code, so it won't be stripped anyway, but proguard will rename it,
    // which this prevents.
    @Override
    @DoNotStrip
    void invokeCallback(
            int callbackID,
            NativeArrayInterface arguments);

    @DoNotStrip
    void callFunction(
            String module,
            String method,
            NativeArray arguments);

    /**
     * Destroys this catalyst instance, waiting for any other threads in ReactQueueConfiguration
     * (besides the UI thread) to finish running. Must be called from the UI thread so that we can
     * fully shut down other threads.
     */
    void destroy();

    boolean isDestroyed();

    /**
     * Initialize all the native modules
     */
    void initialize();

    ReactQueueConfiguration getReactQueueConfiguration();

    <T extends JavaScriptModule> T getJSModule(Class<T> jsInterface);

    <T extends NativeModule> boolean hasNativeModule(Class<T> nativeModuleInterface);

    <T extends NativeModule> T getNativeModule(Class<T> nativeModuleInterface);

    NativeModule getNativeModule(String moduleName);

    <T extends JSIModule> T getJSIModule(Class<T> jsiModuleInterface);

    Collection<NativeModule> getNativeModules();

    /**
     * This method permits a CatalystInstance to extend the known
     * Native modules. This provided registry contains only the new modules to load.
     */
    void extendNativeModules(NativeModuleRegistry modules);


    void setGlobalVariable(String propName, String jsonValue);

    /**
     * Get the C pointer (as a long) to the JavaScriptCore context associated with this instance.
     *
     * <p>Use the following pattern to ensure that the JS context is not cleared while you are using
     * it: JavaScriptContextHolder jsContext = reactContext.getJavaScriptContextHolder()
     * synchronized(jsContext) { nativeThingNeedingJsContext(jsContext.get()); }
     */
    JavaScriptContextHolder getJavaScriptContextHolder();

    void addJSIModules(List<JSIModuleSpec> jsiModules);
}
