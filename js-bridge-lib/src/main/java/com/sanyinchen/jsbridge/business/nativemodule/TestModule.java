package com.sanyinchen.jsbridge.business.nativemodule;

import android.util.Log;

import androidx.annotation.NonNull;

import com.sanyinchen.jsbridge.annotation.ReactMethod;
import com.sanyinchen.jsbridge.base.Promise;
import com.sanyinchen.jsbridge.module.impl.java.BaseJavaModule;


public class TestModule extends BaseJavaModule {
    private LogUpdate logUpdate;


    @NonNull
    @Override
    public String getName() {
        return "TestSum";
    }

    @ReactMethod
    public void sum(int a, int b, Promise promise) {
        Log.d("src_test", "get " + a + " + " + b);
        promise.resolve(a + b);
    }

    public interface LogUpdate {
        void log(String message);
    }
}
