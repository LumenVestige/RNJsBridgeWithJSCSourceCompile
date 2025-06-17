/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * <p>
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.module.impl.java;

import com.facebook.jni.annotations.DoNotStrip;
import com.sanyinchen.jsbridge.annotation.ReactMethod;
import com.sanyinchen.jsbridge.base.JSInstance;
import com.sanyinchen.jsbridge.data.Arguments;
import com.sanyinchen.jsbridge.data.NativeMap;
import com.sanyinchen.jsbridge.data.ReadableNativeArray;
import com.sanyinchen.jsbridge.module.bridge.NativeModule;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleHolder;
import com.sanyinchen.jsbridge.module.bridge.NativeModuleSpec;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.annotation.Nullable;


/**
 * This is part of the glue which wraps a java BaseJavaModule in a C++
 * NativeModule.  This could all be in C++, but it's android-specific
 * initialization code, and writing it this way is easier to read and means
 * fewer JNI calls.
 */

@DoNotStrip
public class JavaModuleWrapper {
    @DoNotStrip
    public class MethodDescriptor {
        @DoNotStrip
        Method method;
        @DoNotStrip
        String signature;
        @DoNotStrip
        String name;
        @DoNotStrip
        String type;
    }

    private final JSInstance mJSInstance;
    private final NativeModuleHolder mNativeModuleHolder;
    private final ArrayList<NativeModule.NativeMethod> mMethods;
    private final ArrayList<MethodDescriptor> mDescs;

    public JavaModuleWrapper(JSInstance jsInstance, NativeModuleHolder NativeModuleHolder) {
        mJSInstance = jsInstance;
        mNativeModuleHolder = NativeModuleHolder;
        mMethods = new ArrayList<>();
        mDescs = new ArrayList();
    }

    @DoNotStrip
    public BaseJavaModule getModule() {
        return (BaseJavaModule) mNativeModuleHolder.getModule();
    }

    @DoNotStrip
    public String getName() {
        return mNativeModuleHolder.getName();
    }

    @DoNotStrip
    private void findMethods() {
        Set<String> methodNames = new HashSet<>();

        Class<? extends NativeModule> classForMethods = mNativeModuleHolder.getModule().getClass();
        Class<? extends NativeModule> superClass =
                (Class<? extends NativeModule>) classForMethods.getSuperclass();
        if (NativeModuleSpec.class.isAssignableFrom(superClass)) {
            // For java module that is based on generated flow-type spec, inspect the
            // spec abstract class instead, which is the super class of the given java
            // module.
            classForMethods = superClass;
        }
        Method[] targetMethods = classForMethods.getDeclaredMethods();

        for (Method targetMethod : targetMethods) {
            ReactMethod annotation = targetMethod.getAnnotation(ReactMethod.class);
            if (annotation != null) {
                String methodName = targetMethod.getName();
                if (methodNames.contains(methodName)) {
                    // We do not support method overloading since js sees a function as an object regardless
                    // of number of params.
                    throw new IllegalArgumentException(
                            "Java Module " + getName() + " method name already registered: " + methodName);
                }
                MethodDescriptor md = new MethodDescriptor();
                JavaMethodWrapper method = new JavaMethodWrapper(this, targetMethod, annotation.isBlockingSynchronousMethod());
                md.name = methodName;
                md.type = method.getType();
                if (md.type == BaseJavaModule.METHOD_TYPE_SYNC) {
                    md.signature = method.getSignature();
                    md.method = targetMethod;
                }
                mMethods.add(method);
                mDescs.add(md);
            }
        }
    }

    @DoNotStrip
    public List<MethodDescriptor> getMethodDescriptors() {
        if (mDescs.isEmpty()) {
            findMethods();
        }
        return mDescs;
    }

    @DoNotStrip
    public @Nullable
    NativeMap getConstants() {
        if (!mNativeModuleHolder.getHasConstants()) {
            return null;
        }

        final String moduleName = getName();
        BaseJavaModule baseJavaModule = getModule();
        Map<String, Object> map = baseJavaModule.getConstants();
        return Arguments.makeNativeMap(map);
    }

    @DoNotStrip
    public void invoke(int methodId, ReadableNativeArray parameters) {
        if (mMethods == null || methodId >= mMethods.size()) {
            return;
        }

        mMethods.get(methodId).invoke(mJSInstance, parameters);
    }
}
