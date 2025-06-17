/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * <p>This source code is licensed under the MIT license found in the LICENSE file in the root
 * directory of this source tree.
 */
package com.sanyinchen.jsbridge.module.bridge;

import com.facebook.infer.annotation.Assertions;
import com.sanyinchen.jsbridge.JsBridgeContext;
import com.sanyinchen.jsbridge.annotation.ReactModule;
import com.sanyinchen.jsbridge.base.JSInstance;
import com.sanyinchen.jsbridge.module.impl.java.JavaModuleWrapper;
import com.sanyinchen.jsbridge.common.callback.OnBatchCompleteListener;


import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/** A set of Java APIs to expose to a particular JavaScript instance. */
public class NativeModuleRegistry {

    private final JsBridgeContext mReactApplicationContext;
    private final Map<String, NativeModuleHolder> mModules;

    public NativeModuleRegistry(
            JsBridgeContext reactApplicationContext, Map<String, NativeModuleHolder> modules) {
        mReactApplicationContext = reactApplicationContext;

        mModules = modules;
        // mModules.remove("NativeLog");
    }

    /** Private getters for combining NativeModuleRegistrys */
    private Map<String, NativeModuleHolder> getModuleMap() {
        return mModules;
    }

    private JsBridgeContext getReactApplicationContext() {
        return mReactApplicationContext;
    }

    public Collection<JavaModuleWrapper> getJavaModules(JSInstance jsInstance) {
        ArrayList<JavaModuleWrapper> javaModules = new ArrayList<>();
        for (Map.Entry<String, NativeModuleHolder> entry : mModules.entrySet()) {
            if (!entry.getValue().isCxxModule()) {
                javaModules.add(new JavaModuleWrapper(jsInstance, entry.getValue()));
            }
        }
        return javaModules;
    }

    public Collection<NativeModuleHolder> getCxxModules() {
        ArrayList<NativeModuleHolder> cxxModules = new ArrayList<>();
        for (Map.Entry<String, NativeModuleHolder> entry : mModules.entrySet()) {
            if (entry.getValue().isCxxModule()) {
                cxxModules.add(entry.getValue());
            }
        }
        return cxxModules;
    }

    /*
     * Adds any new modules to the current module registry
     */
    public void registerModules(NativeModuleRegistry newRegister) {

        Assertions.assertCondition(
                mReactApplicationContext.equals(newRegister.getReactApplicationContext()),
                "Extending native modules with non-matching application contexts.");

        Map<String, NativeModuleHolder> newModules = newRegister.getModuleMap();

        for (Map.Entry<String, NativeModuleHolder> entry : newModules.entrySet()) {
            String key = entry.getKey();
            if (!mModules.containsKey(key)) {
                NativeModuleHolder value = entry.getValue();
                mModules.put(key, value);
            }
        }
    }

    public void notifyJSInstanceDestroy() {
        mReactApplicationContext.assertOnNativeModulesQueueThread();
        for (NativeModuleHolder module : mModules.values()) {
            module.destroy();
        }
    }

    public void notifyJSInstanceInitialized() {
        mReactApplicationContext.assertOnNativeModulesQueueThread(
                "From version React Native v0.44, "
                        + "native modules are explicitly not initialized on the UI thread. See "
                        + "https://github.com/facebook/react-native/wiki/Breaking-Changes#d4611211-reactnativeandroidbreaking-move-nativemodule-initialization-off-ui-thread---aaachiuuu "
                        + " for more details.");

        for (NativeModuleHolder module : mModules.values()) {
            module.markInitializable();
        }
    }

    public void onBatchComplete() {
        // The only native module that uses the onBatchComplete is the UI Manager. Hence, instead of
        // iterating over all the modules for find this one instance, and then calling it, we
        // short-circuit
        // the search, and simply call OnBatchComplete on the UI Manager.
        // With Fabric, UIManager would no longer be a NativeModule, so this call would simply go away
        NativeModuleHolder moduleHolder = mModules.get("UIManager");
        if (moduleHolder != null && moduleHolder.hasInstance()) {
            ((OnBatchCompleteListener) moduleHolder.getModule()).onBatchComplete();
        }
    }

    public <T extends NativeModule> boolean hasModule(Class<T> moduleInterface) {
        String name = moduleInterface.getAnnotation(ReactModule.class).name();
        return mModules.containsKey(name);
    }

    public <T extends NativeModule> T getModule(Class<T> moduleInterface) {
        ReactModule annotation = moduleInterface.getAnnotation(ReactModule.class);
        if (annotation == null) {
            throw new IllegalArgumentException(
                    "Could not find @ReactModule annotation in class " + moduleInterface.getName());
        }
        return (T)
                Assertions.assertNotNull(
                                mModules.get(annotation.name()),
                                annotation.name()
                                        + " could not be found. Is it defined in "
                                        + moduleInterface.getName())
                        .getModule();
    }

    public boolean hasModule(String name) {
        return mModules.containsKey(name);
    }

    public NativeModule getModule(String name) {
        return Assertions.assertNotNull(
                mModules.get(name), "Could not find module with name " + name).getModule();
    }

    public List<NativeModule> getAllModules() {
        List<NativeModule> modules = new ArrayList<>();
        for (NativeModuleHolder module : mModules.values()) {
            modules.add(module.getModule());
        }
        return modules;
    }
}
