/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.exception;


import com.facebook.jni.annotations.DoNotStrip;

@DoNotStrip
public class CppException extends RuntimeException {
  @DoNotStrip
  public CppException(String message) {
    super(message);
  }
}
