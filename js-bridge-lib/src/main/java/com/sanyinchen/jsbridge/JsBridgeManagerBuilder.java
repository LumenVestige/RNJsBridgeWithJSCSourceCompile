// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

package com.sanyinchen.jsbridge;

import android.app.Activity;
import android.app.Application;

import com.facebook.infer.annotation.Assertions;
import com.sanyinchen.jsbridge.exception.NativeModuleCallExceptionHandler;
import com.sanyinchen.jsbridge.executor.base.JavaScriptExecutorFactory;
import com.sanyinchen.jsbridge.executor.jsc.JSCExecutorFactory;
import com.sanyinchen.jsbridge.load.JSBundleLoader;
import com.sanyinchen.jsbridge.module.bridge.NativeModelPackage;
import com.sanyinchen.jsbridge.module.jsi.JSIModulePackage;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nullable;

/**
 * Builder class for {@link JsBridgeManager}
 */
public class JsBridgeManagerBuilder {

    private final List<NativeModelPackage> mPackages = new ArrayList<>();

    private @Nullable
    String mJSBundleAssetUrl;
    private @Nullable
    JSBundleLoader mJSBundleLoader;
    private @Nullable
    String mJSMainModulePath;
    private @Nullable
    Application mApplication;
    private boolean mUseDeveloperSupport;
    private @Nullable
    NativeModuleCallExceptionHandler mNativeModuleCallExceptionHandler;
    private @Nullable
    Activity mCurrentActivity;
    private boolean mLazyViewManagersEnabled;
    private @Nullable
    JavaScriptExecutorFactory mJavaScriptExecutorFactory;
    private int mMinNumShakes = 1;
    private int mMinTimeLeftInFrameForNonBatchedOperationMs = -1;
    private @Nullable
    JSIModulePackage mJSIModulesPackage;

    public JsBridgeManagerBuilder() {
    }


    public JsBridgeManagerBuilder setJSIModulesPackage(
            @Nullable JSIModulePackage jsiModulePackage) {
        mJSIModulesPackage = jsiModulePackage;
        return this;
    }

    /**
     * Factory for desired implementation of JavaScriptExecutor.
     */
    public JsBridgeManagerBuilder setJavaScriptExecutorFactory(
            @Nullable JavaScriptExecutorFactory javaScriptExecutorFactory) {
        mJavaScriptExecutorFactory = javaScriptExecutorFactory;
        return this;
    }

    /**
     * Name of the JS bundle file to be loaded from application's raw assets.
     * Example: {@code "index.android.js"}
     */
    public JsBridgeManagerBuilder setBundleAssetName(String bundleAssetName) {
        mJSBundleAssetUrl = (bundleAssetName == null ? null : "assets://" + bundleAssetName);
        mJSBundleLoader = null;
        return this;
    }


    /**
     * Bundle loader to use when setting up JS environment. This supersedes
     * prior invocations of {@link setJSBundleFile} and {@link setBundleAssetName}.
     * <p>
     * Example: {@code JSBundleLoader.createFileLoader(application, bundleFile)}
     */
    public JsBridgeManagerBuilder setJSBundleLoader(JSBundleLoader jsBundleLoader) {
        mJSBundleLoader = jsBundleLoader;
        mJSBundleAssetUrl = null;
        return this;
    }

    /**
     * Path to your app's main module on the packager server. This is used when
     * reloading JS during development. All paths are relative to the root folder
     * the packager is serving files from.
     * Examples:
     * {@code "index.android"} or
     * {@code "subdirectory/index.android"}
     */
    public JsBridgeManagerBuilder setJSMainModulePath(String jsMainModulePath) {
        mJSMainModulePath = jsMainModulePath;
        return this;
    }

    public JsBridgeManagerBuilder addPackage(NativeModelPackage reactPackage) {
        mPackages.add(reactPackage);
        return this;
    }

    public JsBridgeManagerBuilder addPackages(List<NativeModelPackage> reactPackages) {
        mPackages.addAll(reactPackages);
        return this;
    }

    /**
     * Required. This must be your {@code Application} instance.
     */
    public JsBridgeManagerBuilder setApplication(Application application) {
        mApplication = application;
        return this;
    }

    public JsBridgeManagerBuilder setCurrentActivity(Activity activity) {
        mCurrentActivity = activity;
        return this;
    }


    /**
     * Set the exception handler for all native module calls. If not set, the default
     * {@link DevSupportManager} will be used, which shows a redbox in dev mode and rethrows
     * (crashes the app) in prod mode.
     */
    public JsBridgeManagerBuilder setNativeModuleCallExceptionHandler(
            NativeModuleCallExceptionHandler handler) {
        mNativeModuleCallExceptionHandler = handler;
        return this;
    }


    public JsBridgeManagerBuilder setLazyViewManagersEnabled(boolean lazyViewManagersEnabled) {
        mLazyViewManagersEnabled = lazyViewManagersEnabled;
        return this;
    }


    /**
     * Instantiates a new {@link ReactInstanceManager}.
     * Before calling {@code build}, the following must be called:
     * <ul>
     * <li> {@link #setApplication}
     * <li> {@link #setCurrentActivity} if the activity has already resumed
     * <li> {@link #setDefaultHardwareBackBtnHandler} if the activity has already resumed
     * <li> {@link #setJSBundleFile} or {@link #setJSMainModulePath}
     * </ul>
     */
    public JsBridgeManager build() {
        Assertions.assertNotNull(
                mApplication,
                "Application property has not been set with this builder");

        Assertions.assertCondition(
                mUseDeveloperSupport || mJSBundleAssetUrl != null || mJSBundleLoader != null,
                "JS Bundle File or Asset URL has to be provided when dev support is disabled");

        Assertions.assertCondition(
                mJSMainModulePath != null || mJSBundleAssetUrl != null || mJSBundleLoader != null,
                "Either MainModulePath or JS Bundle File needs to be provided");

        // We use the name of the device and the app for debugging & metrics
        String appName = mApplication.getPackageName();

        return new JsBridgeManager(
                mApplication,
                mCurrentActivity,
                mJavaScriptExecutorFactory == null
                        ? new JSCExecutorFactory(appName, "android")
                        : mJavaScriptExecutorFactory,
                (mJSBundleLoader == null && mJSBundleAssetUrl != null)
                        ? JSBundleLoader.createAssetLoader(
                        mApplication, mJSBundleAssetUrl, false /*Asynchronous*/)
                        : mJSBundleLoader,
                mJSMainModulePath,
                mPackages,
                mNativeModuleCallExceptionHandler,
                mJSIModulesPackage);
    }
}
