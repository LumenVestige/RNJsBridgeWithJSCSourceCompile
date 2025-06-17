// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

package com.sanyinchen.jsbridge.module.impl.cxx;

import static com.sanyinchen.jsbridge.data.Arguments.fromJavaArgs;

import com.facebook.jni.HybridData;
import com.facebook.jni.annotations.DoNotStrip;
import com.sanyinchen.jsbridge.base.Callback;
import com.sanyinchen.jsbridge.data.NativeArray;

/**
 * Callback impl that calls directly into the cxx bridge. Created from C++.
 */
@DoNotStrip
public class CxxCallbackImpl implements Callback {
  @DoNotStrip
  private final HybridData mHybridData;

  @DoNotStrip
  private CxxCallbackImpl(HybridData hybridData) {
    mHybridData = hybridData;
  }

  @Override
  public void invoke(Object... args) {
    nativeInvoke(fromJavaArgs(args));
  }

  private native void nativeInvoke(NativeArray arguments);
}
