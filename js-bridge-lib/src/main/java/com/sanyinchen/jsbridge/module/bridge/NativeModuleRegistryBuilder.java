// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

package com.sanyinchen.jsbridge.module.bridge;

import com.sanyinchen.jsbridge.JsBridgeContext;
import com.sanyinchen.jsbridge.JsBridgeManager;
import com.sanyinchen.jsbridge.config.ReactFeatureFlags;

import java.util.HashMap;
import java.util.Map;

/** Helper class to build NativeModuleRegistry. */
public class NativeModuleRegistryBuilder {

    private final JsBridgeContext mReactApplicationContext;
    private final JsBridgeManager mReactInstanceManager;

    private final Map<String, NativeModuleHolder> mModules = new HashMap<>();

    public NativeModuleRegistryBuilder(
            JsBridgeContext reactApplicationContext, JsBridgeManager reactInstanceManager) {
        mReactApplicationContext = reactApplicationContext;
        mReactInstanceManager = reactInstanceManager;
    }

    public void processPackage(NativeModelPackage reactPackage) {
        // We use an iterable instead of an iterator here to ensure thread safety, and that this list
        // cannot be modified
        Iterable<NativeModuleHolder> moduleHolders =
                ReactPackageHelper.getNativeModuleIterator(
                        reactPackage, mReactApplicationContext, mReactInstanceManager);

        for (NativeModuleHolder moduleHolder : moduleHolders) {
            String name = moduleHolder.getName();
            if (mModules.containsKey(name)) {
                NativeModuleHolder existingNativeModule = mModules.get(name);
                if (!moduleHolder.getCanOverrideExistingModule()) {
                    throw new IllegalStateException(
                            "Native module "
                                    + name
                                    + " tried to override "
                                    + existingNativeModule.getClassName()
                                    + " for module name .Check the getPackages() method in MainApplication.java, it might be that module is being created twice. If this was your intention, set canOverrideExistingModule=true");
                }
                mModules.remove(existingNativeModule);
            }
            if (ReactFeatureFlags.useTurboModules && moduleHolder.isTurboModule()) {
                // If this module is a TurboModule, and if TurboModules are enabled, don't add this module

                // This condition is after checking for overrides, since if there is already a module,
                // and we want to override it with a turbo module, we would need to remove the modules thats
                // already in the list, and then NOT add the new module, since that will be directly exposed

                // Note that is someone uses {@link NativeModuleRegistry#registerModules}, we will NOT check
                // for TurboModules - assuming that people wanted to explicitly register native modules there
                continue;
            }
            mModules.put(name, moduleHolder);
        }
    }

    public NativeModuleRegistry build() {
        return new NativeModuleRegistry(mReactApplicationContext, mModules);
    }
}
