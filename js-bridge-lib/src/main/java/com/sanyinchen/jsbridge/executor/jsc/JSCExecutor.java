/**
 * Copyright (c) 2015-present, Facebook, Inc.
 *
 * <p>This source code is licensed under the MIT license found in the LICENSE file in the root
 * directory of this source tree.
 */

package com.sanyinchen.jsbridge.executor.jsc;

import com.facebook.jni.HybridData;
import com.facebook.jni.annotations.DoNotStrip;
import com.facebook.soloader.SoLoader;
import com.sanyinchen.jsbridge.data.ReadableNativeMap;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutor;

@DoNotStrip
        /* package */ class JSCExecutor extends JavaScriptExecutor {
    static {
        SoLoader.loadLibrary("jscexecutor");
    }

    /* package */ JSCExecutor(ReadableNativeMap jscConfig) {
        super(initHybrid(jscConfig));
    }

    @Override
    public String getName() {
        return "JSCExecutor";
    }

    private static native HybridData initHybrid(ReadableNativeMap jscConfig);
}
