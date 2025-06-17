/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.module.jsi;

import com.sanyinchen.jsbridge.JsBridgeContext;
import com.sanyinchen.jsbridge.context.JavaScriptContextHolder;

import java.util.List;

/**
 * Marker interface used to represent a JSI Module.
 */
public interface JSIModule {

    /**
     * This is called at the end of {@link CatalystApplicationFragment#createCatalystInstance()}
     * after the CatalystInstance has been created, in order to initialize NativeModules that require
     * the CatalystInstance or JS modules.
     */
    void initialize();

    /**
     * Called before {CatalystInstance#onHostDestroy}
     */
    void onCatalystInstanceDestroy();
}
