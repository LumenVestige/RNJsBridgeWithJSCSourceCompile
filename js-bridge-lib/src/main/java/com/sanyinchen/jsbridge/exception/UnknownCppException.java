/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.exception;

import com.facebook.jni.CppException;
import com.facebook.jni.annotations.DoNotStrip;

@DoNotStrip
public class UnknownCppException extends CppException {
  @DoNotStrip
  public UnknownCppException() {
    super("Unknown");
  }

  @DoNotStrip
  public UnknownCppException(String message) {
    super(message);
  }
}
