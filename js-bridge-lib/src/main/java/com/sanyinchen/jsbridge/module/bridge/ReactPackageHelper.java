/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.module.bridge;


import androidx.annotation.NonNull;


import com.sanyinchen.jsbridge.JsBridgeManager;
import com.sanyinchen.jsbridge.config.ReactConstants;
import com.sanyinchen.jsbridge.JsBridgeContext;
import com.sanyinchen.jsbridge.utils.log.FLog;

import java.util.Iterator;
import java.util.List;

public class ReactPackageHelper {
    /**
     * A helper method to iterate over a list of Native Modules and convert them to an iterable.
     *
     * @param reactPackage
     * @param reactApplicationContext
     * @param reactInstanceManager
     * @return
     */
    public static Iterable<NativeModuleHolder> getNativeModuleIterator(
            NativeModelPackage reactPackage,
            JsBridgeContext reactApplicationContext,
            JsBridgeManager reactInstanceManager) {
        FLog.d(
                ReactConstants.TAG,
                reactPackage.getClass().getSimpleName()
                        + " is not a LazyReactPackage, falling back to old version.");
        final List<NativeModule> nativeModules = reactPackage.createNativeModules(reactApplicationContext);
        return new Iterable<NativeModuleHolder>() {
            @NonNull
            @Override
            public Iterator<NativeModuleHolder> iterator() {
                return new Iterator<NativeModuleHolder>() {
                    int position = 0;

                    @Override
                    public NativeModuleHolder next() {
                        return new NativeModuleHolder(nativeModules.get(position++));
                    }

                    @Override
                    public boolean hasNext() {
                        return position < nativeModules.size();
                    }

                    @Override
                    public void remove() {
                        throw new UnsupportedOperationException("Cannot remove methods ");
                    }
                };
            }
        };
    }
}
