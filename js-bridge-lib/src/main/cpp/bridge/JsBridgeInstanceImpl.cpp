// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "JsBridgeInstanceImpl.h"

#include <mutex>
#include <condition_variable>
#include <sstream>
#include <vector>

#include <CxxNativeModule.h>
#include <Instance.h>
#include <JSBigString.h>
#include <JSBundleType.h>
#include <JSIndexedRAMBundle.h>
#include <MethodCall.h>
#include <ModuleRegistry.h>
#include <RecoverableError.h>
#include <RAMBundleRegistry.h>
#include <fbjni/fbjni.h>
#include <folly/dynamic.h>
#include <folly/Memory.h>

#include "CxxModuleWrapper.h"
#include "bridge/include/JavaScriptExecutorHolder.h"
#include "JNativeRunnable.h"
#include "JniJSModulesUnbundle.h"
#include "NativeArray.h"
#include <TraceSection.h>
using namespace facebook::jni;

namespace facebook {
    namespace react {

        namespace {

            class Exception : public jni::JavaClass<Exception> {
            public:
            };

            class JInstanceCallback : public InstanceCallback {
            public:
                explicit JInstanceCallback(
                        alias_ref <ReactCallback::javaobject> jobj,
                        std::shared_ptr <JMessageQueueThread> messageQueueThread)
                        : jobj_(make_global(jobj)),
                          messageQueueThread_(std::move(messageQueueThread)) {}

                void onBatchComplete() override {
                    messageQueueThread_->runOnQueue([this] {
                        static auto method =
                                ReactCallback::javaClassStatic()->getMethod<void()>(
                                        "onBatchComplete");
                        method(jobj_);
                    });
                }

                void incrementPendingJSCalls() override {
                    // For C++ modules, this can be called from an arbitrary thread
                    // managed by the module, via callJSCallback or callJSFunction.  So,
                    // we ensure that it is registered with the JVM.
                    jni::ThreadScope guard;
                    static auto method =
                            ReactCallback::javaClassStatic()->getMethod<void()>(
                                    "incrementPendingJSCalls");
                    method(jobj_);
                }

                void decrementPendingJSCalls() override {
                    jni::ThreadScope guard;
                    static auto method =
                            ReactCallback::javaClassStatic()->getMethod<void()>(
                                    "decrementPendingJSCalls");
                    method(jobj_);
                }

            private:
                global_ref <ReactCallback::javaobject> jobj_;
                std::shared_ptr <JMessageQueueThread> messageQueueThread_;
            };

        }

        jni::local_ref <JsBridgeInstanceImpl::jhybriddata> JsBridgeInstanceImpl::initHybrid(
                jni::alias_ref <jclass>) {
            TRACE_SECTION("JsBridgeInstanceImpl::initHybrid");
            return makeCxxInstance();
        }

        JsBridgeInstanceImpl::JsBridgeInstanceImpl()
                : instance_(folly::make_unique<Instance>()) {}

        JsBridgeInstanceImpl::~JsBridgeInstanceImpl() {
            if (moduleMessageQueue_ != NULL) {
                moduleMessageQueue_->quitSynchronous();
            }
        }

        void JsBridgeInstanceImpl::registerNatives() {
            registerHybrid({
                                   makeNativeMethod("initHybrid", JsBridgeInstanceImpl::initHybrid),
                                   makeNativeMethod("initializeBridge",
                                                    JsBridgeInstanceImpl::initializeBridge),
                                   makeNativeMethod("jniExtendNativeModules",
                                                    JsBridgeInstanceImpl::extendNativeModules),
                                   makeNativeMethod("jniLoadScriptFromAssets",
                                                    JsBridgeInstanceImpl::jniLoadScriptFromAssets),
                                   makeNativeMethod("jniCallJSFunction",
                                                    JsBridgeInstanceImpl::jniCallJSFunction),
                                   makeNativeMethod("jniCallJSCallback",
                                                    JsBridgeInstanceImpl::jniCallJSCallback),
                                   makeNativeMethod("setGlobalVariable",
                                                    JsBridgeInstanceImpl::setGlobalVariable),
                                   makeNativeMethod("getJavaScriptContext",
                                                    JsBridgeInstanceImpl::getJavaScriptContext),
                                   makeNativeMethod("jniHandleMemoryPressure",
                                                    JsBridgeInstanceImpl::handleMemoryPressure),
                           });

            JNativeRunnable::registerNatives();
        }

        void JsBridgeInstanceImpl::initializeBridge(
                jni::alias_ref <ReactCallback::javaobject> callback,
                // This executor is actually a factory holder.
                JavaScriptExecutorHolder *jseh,
                jni::alias_ref <JavaMessageQueueThread::javaobject> jsQueue,
                jni::alias_ref <JavaMessageQueueThread::javaobject> nativeModulesQueue,
                jni::alias_ref <jni::JCollection<JavaModuleWrapper::javaobject>::javaobject> javaModules,
                jni::alias_ref <jni::JCollection<ModuleHolder::javaobject>::javaobject> cxxModules) {
            TRACE_SECTION("JsBridgeInstanceImpl::initializeBridge");
            // TODO mhorowitz: how to assert here?
            // Assertions.assertCondition(mBridge == null, "initializeBridge should be called once");
            moduleMessageQueue_ = std::make_shared<JMessageQueueThread>(nativeModulesQueue);

            // This used to be:
            //
            // Java JsBridgeInstanceImpl -> C++ JsBridgeInstanceImpl -> Bridge -> Bridge::Callback
            // --weak--> ReactCallback -> Java JsBridgeInstanceImpl
            //
            // Now the weak ref is a global ref.  So breaking the loop depends on
            // JsBridgeInstanceImpl#destroy() calling mHybridData.resetNative(), which
            // should cause all the C++ pointers to be cleaned up (except C++
            // JsBridgeInstanceImpl might be kept alive for a short time by running
            // callbacks). This also means that all native calls need to be pre-checked
            // to avoid NPE.

            // See the comment in callJSFunction.  Once js calls switch to strings, we
            // don't need jsModuleDescriptions any more, all the way up and down the
            // stack.

            moduleRegistry_ = std::make_shared<ModuleRegistry>(
                    buildNativeModuleList(
                            std::weak_ptr<Instance>(instance_),
                            javaModules,
                            cxxModules,
                            moduleMessageQueue_));

            instance_->initializeBridge(
                    std::make_unique<JInstanceCallback>(
                            callback,
                            moduleMessageQueue_),
                    jseh->getExecutorFactory(),
                    folly::make_unique<JMessageQueueThread>(jsQueue),
                    moduleRegistry_);
        }

        void JsBridgeInstanceImpl::extendNativeModules(
                jni::alias_ref <jni::JCollection<JavaModuleWrapper::javaobject>::javaobject> javaModules,
                jni::alias_ref <jni::JCollection<ModuleHolder::javaobject>::javaobject> cxxModules) {
            TRACE_SECTION("JsBridgeInstanceImpl::extendNativeModules");
            moduleRegistry_->registerModules(buildNativeModuleList(
                    std::weak_ptr<Instance>(instance_),
                    javaModules,
                    cxxModules,
                    moduleMessageQueue_));
        }


        void JsBridgeInstanceImpl::jniLoadScriptFromAssets(
                jni::alias_ref <JAssetManager::javaobject> assetManager,
                const std::string &assetURL,
                bool loadSynchronously) {
            TRACE_SECTION("JsBridgeInstanceImpl::jniLoadScriptFromAssets");
            const int kAssetsLength = 9;  // strlen("assets://");
            auto sourceURL = assetURL.substr(kAssetsLength);
            auto manager = extractAssetManager(assetManager);
            auto script = loadScriptFromAssets(manager, sourceURL);
            if (JniJSModulesUnbundle::isUnbundle(manager, sourceURL)) {
                auto bundle = JniJSModulesUnbundle::fromEntryFile(manager, sourceURL);
                auto registry = RAMBundleRegistry::singleBundleRegistry(std::move(bundle));
                instance_->loadRAMBundle(
                        std::move(registry),
                        std::move(script),
                        sourceURL,
                        loadSynchronously);
                return;
            } else {
                instance_->loadScriptFromString(std::move(script), sourceURL, loadSynchronously);
            }
        }


        void JsBridgeInstanceImpl::jniCallJSFunction(std::string module, std::string method,
                                                     NativeArray *arguments) {
            TRACE_SECTION("JsBridgeInstanceImpl::jniCallJSFunction");
            // We want to share the C++ code, and on iOS, modules pass module/method
            // names as strings all the way through to JS, and there's no way to do
            // string -> id mapping on the objc side.  So on Android, we convert the
            // number to a string, here which gets passed as-is to JS.  There, they they
            // used as ids if isFinite(), which handles this case, and looked up as
            // strings otherwise.  Eventually, we'll probably want to modify the stack
            // from the JS proxy through here to use strings, too.
            instance_->callJSFunction(std::move(module),
                                      std::move(method),
                                      arguments->consume());
        }

        void JsBridgeInstanceImpl::jniCallJSCallback(jint callbackId, NativeArray *arguments) {
            TRACE_SECTION("JsBridgeInstanceImpl::jniCallJSCallback");
            instance_->callJSCallback(callbackId, arguments->consume());
        }

        void JsBridgeInstanceImpl::setGlobalVariable(std::string propName,
                                                     std::string &&jsonValue) {
            TRACE_SECTION("JsBridgeInstanceImpl::setGlobalVariable");
            // This is only ever called from Java with short strings, and only
            // for testing, so no need to try hard for zero-copy here.

            instance_->setGlobalVariable(std::move(propName),
                                         folly::make_unique<JSBigStdString>(std::move(jsonValue)));
        }

        jlong JsBridgeInstanceImpl::getJavaScriptContext() {
            return (jlong)(intptr_t)
            instance_->getJavaScriptContext();
        }

        void JsBridgeInstanceImpl::handleMemoryPressure(int pressureLevel) {
            instance_->handleMemoryPressure(pressureLevel);
        }

    }
}
