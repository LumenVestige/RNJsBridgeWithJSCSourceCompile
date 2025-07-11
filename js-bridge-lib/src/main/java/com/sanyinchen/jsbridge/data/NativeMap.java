/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.data;

import com.facebook.jni.HybridData;
import com.facebook.jni.annotations.DoNotStrip;

/**
 * Base class for a Map whose keys and values are stored in native code (C++).
 */
@DoNotStrip
public abstract class NativeMap {

    public NativeMap(HybridData hybridData) {
        mHybridData = hybridData;
    }

    @Override
    public native String toString();

    @DoNotStrip
    private HybridData mHybridData;
}
