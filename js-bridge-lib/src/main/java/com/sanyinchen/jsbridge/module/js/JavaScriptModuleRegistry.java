/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.module.js;


import androidx.annotation.Nullable;

import com.sanyinchen.jsbridge.base.JsBridgeInstance;
import com.sanyinchen.jsbridge.config.ReactBuildConfig;
import com.sanyinchen.jsbridge.data.Arguments;
import com.sanyinchen.jsbridge.data.NativeArray;
import com.sanyinchen.jsbridge.data.WritableNativeArray;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

/**
 * Class responsible for holding all the {@link JavaScriptModule}s.  Uses Java proxy objects
 * to dispatch method calls on JavaScriptModules to the bridge using the corresponding
 * module and method ids so the proper function is executed in JavaScript.
 */
public final class JavaScriptModuleRegistry {
    private final HashMap<Class<? extends JavaScriptModule>, JavaScriptModule> mModuleInstances;

    public JavaScriptModuleRegistry() {
        mModuleInstances = new HashMap<>();
    }

    public synchronized <T extends JavaScriptModule> T getJavaScriptModule(
            JsBridgeInstance instance,
            Class<T> moduleInterface) {
        JavaScriptModule module = mModuleInstances.get(moduleInterface);
        if (module != null) {
            return (T) module;
        }

        JavaScriptModule interfaceProxy = (JavaScriptModule) Proxy.newProxyInstance(
                moduleInterface.getClassLoader(),
                new Class[]{moduleInterface},
                new JavaScriptModuleInvocationHandler(instance, moduleInterface));
        mModuleInstances.put(moduleInterface, interfaceProxy);
        return (T) interfaceProxy;
    }

    private static class JavaScriptModuleInvocationHandler implements InvocationHandler {
        private final JsBridgeInstance mJsBridgeInstance;
        private final Class<? extends JavaScriptModule> mModuleInterface;
        private @Nullable
        String mName;

        public JavaScriptModuleInvocationHandler(
                JsBridgeInstance JsBridgeInstance,
                Class<? extends JavaScriptModule> moduleInterface) {
            mJsBridgeInstance = JsBridgeInstance;
            mModuleInterface = moduleInterface;

            if (ReactBuildConfig.DEBUG) {
                Set<String> methodNames = new HashSet<>();
                for (Method method : mModuleInterface.getDeclaredMethods()) {
                    if (!methodNames.add(method.getName())) {
                        throw new AssertionError(
                                "Method overloading is unsupported: " + mModuleInterface.getName() +
                                        "#" + method.getName());
                    }
                }
            }
        }

        private String getJSModuleName() {
            if (mName == null) {
                // With proguard obfuscation turned on, proguard apparently (poorly) emulates inner
                // classes or something because Class#getSimpleName() no longer strips the outer
                // class name. We manually strip it here if necessary.
                String name = mModuleInterface.getSimpleName();
                int dollarSignIndex = name.lastIndexOf('$');
                if (dollarSignIndex != -1) {
                    name = name.substring(dollarSignIndex + 1);
                }

                // getting the class name every call is expensive, so cache it
                mName = name;
            }
            return mName;
        }

        @Override
        public @Nullable
        Object invoke(Object proxy, Method method, @Nullable Object[] args) throws Throwable {
            NativeArray jsArgs = args != null
                    ? Arguments.fromJavaArgs(args)
                    : new WritableNativeArray();
            mJsBridgeInstance.callFunction(getJSModuleName(), method.getName(), jsArgs);
            return null;
        }
    }
}
