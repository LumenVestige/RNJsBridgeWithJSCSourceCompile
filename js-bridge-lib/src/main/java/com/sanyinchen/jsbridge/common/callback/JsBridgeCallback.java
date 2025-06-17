/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.common.callback;


import com.facebook.jni.annotations.DoNotStrip;

@DoNotStrip
public interface JsBridgeCallback {
    @DoNotStrip
    void onBatchComplete();

    @DoNotStrip
    void incrementPendingJSCalls();

    @DoNotStrip
    void decrementPendingJSCalls();
}
