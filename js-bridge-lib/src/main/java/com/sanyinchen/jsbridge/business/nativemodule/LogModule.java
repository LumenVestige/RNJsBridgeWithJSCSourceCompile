package com.sanyinchen.jsbridge.business.nativemodule;

import android.util.Log;

import androidx.annotation.NonNull;

import com.sanyinchen.jsbridge.annotation.ReactMethod;
import com.sanyinchen.jsbridge.module.impl.java.BaseJavaModule;


public class LogModule extends BaseJavaModule {
    private LogUpdate logUpdate;

    public LogModule(LogUpdate logUpdate) {
        this.logUpdate = logUpdate;
    }

    @NonNull
    @Override
    public String getName() {
        return "NativeLog";
    }

    @ReactMethod
    public void log(String message) {
        Log.d("src_test", "message:" + message);
        if (logUpdate != null) {
            logUpdate.log(message);
        }
    }

    public interface LogUpdate {
        void log(String message);
    }
}
