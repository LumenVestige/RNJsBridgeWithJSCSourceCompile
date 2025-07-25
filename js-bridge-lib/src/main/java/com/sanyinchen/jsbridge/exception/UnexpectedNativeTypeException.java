/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.exception;

import com.facebook.jni.annotations.DoNotStrip;

/**
 * Exception thrown from native code when a type retrieved from a map or array (e.g. via
 * {@link NativeArrayParameter#getString(int)}) does not match the expected type.
 */
@DoNotStrip
public class UnexpectedNativeTypeException extends RuntimeException {

  @DoNotStrip
  public UnexpectedNativeTypeException(String msg) {
    super(msg);
  }
}
