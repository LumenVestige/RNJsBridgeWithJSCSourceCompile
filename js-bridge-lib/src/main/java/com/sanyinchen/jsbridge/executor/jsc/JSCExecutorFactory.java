/*
 * Copyright (c) 2015-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.executor.jsc;

import com.sanyinchen.jsbridge.data.WritableNativeMap;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutor;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutorFactory;

public class JSCExecutorFactory implements JavaScriptExecutorFactory {
    private final String mAppName;
    private final String mDeviceName;

    public JSCExecutorFactory(String appName, String deviceName) {
        this.mAppName = appName;
        this.mDeviceName = deviceName;
    }

    @Override
    public JavaScriptExecutor create() throws Exception {
        WritableNativeMap jscConfig = new WritableNativeMap();
        jscConfig.putString("OwnerIdentity", "ReactNative");
        jscConfig.putString("AppIdentity", mAppName);
        jscConfig.putString("DeviceIdentity", mDeviceName);
        return new JSCExecutor(jscConfig);
    }

    @Override
    public String toString() {
        return "JSIExecutor+JSCRuntime";
    }
}
