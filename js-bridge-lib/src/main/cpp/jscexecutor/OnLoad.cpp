//  Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <fbjni/fbjni.h>
#include <folly/Memory.h>
#include "jscexecutor/jsi/jsi/JSCRuntime.h"
#include "JSExecutor.h"
#include <jsiexecutor/JSIExecutor.h>
#include "bridge/include/JavaScriptExecutorHolder.h"
#include "bridge/include/JSLogging.h"
#include <ReadableNativeMap.h>

namespace facebook {
    namespace react {

        namespace {

            class JSCExecutorFactory : public JSExecutorFactory {
            public:
                std::unique_ptr<JSExecutor> createJSExecutor(
                        std::shared_ptr<ExecutorDelegate> delegate,
                        std::shared_ptr<MessageQueueThread> jsQueue) override {
                    return folly::make_unique<JSIExecutor>(
                            jsc::makeJSCRuntime(),
                            delegate,
                            [](const std::string &message, unsigned int logLevel) {
                                reactAndroidLoggingHook(message, logLevel);
                            },
                            JSIExecutor::defaultTimeoutInvoker,
                            nullptr);
                }
            };

        }

// This is not like JSCJavaScriptExecutor, which calls JSC directly.  This uses
// JSIExecutor with JSCRuntime.
        class JSCExecutorHolder
                : public jni::HybridClass<JSCExecutorHolder, JavaScriptExecutorHolder> {
        public:
            static constexpr auto kJavaDescriptor = "Lcom/sanyinchen/jsbridge/executor/jsc/JSCExecutor;";

            static jni::local_ref<jhybriddata> initHybrid(
                    jni::alias_ref<jclass>, ReadableNativeMap *) {
                return makeCxxInstance(folly::make_unique<JSCExecutorFactory>());
            }

            static void registerNatives() {
                registerHybrid({
                                       makeNativeMethod("initHybrid", JSCExecutorHolder::initHybrid),
                               });
            }

        private:
            friend HybridBase;
            using HybridBase::HybridBase;
        };

    } // namespace react
} // namespace facebook

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    return facebook::jni::initialize(vm, [] {
        facebook::react::JSCExecutorHolder::registerNatives();
    });
}
