/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * <p>
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.module.bridge;

import androidx.annotation.NonNull;

import com.sanyinchen.jsbridge.JsBridgeContext;

import java.util.List;

/**
 * Main interface for providing additional capabilities to the catalyst framework by couple of
 * different means:
 * 1) Registering new native modules
 * 2) Registering new JS modules that may be accessed from native modules or from other parts of the
 * native code (requiring JS modules from the package doesn't mean it will automatically be included
 * as a part of the JS bundle, so there should be a corresponding piece of code on JS side that will
 * require implementation of that JS module so that it gets bundled)
 * 3) Registering custom native views (view managers) and custom event types
 * 4) Registering natively packaged assets/resources (e.g. images) exposed to JS
 * <p>
 * TODO(6788500, 6788507): Implement support for adding custom views, events and resources
 */
public interface NativeModelPackage {

    /**
     * @param reactContext react application context that can be used to create modules
     * @return list of native modules to register with the newly created catalyst instance
     */
    @NonNull
    List<NativeModule> createNativeModules(@NonNull JsBridgeContext reactContext);

}
