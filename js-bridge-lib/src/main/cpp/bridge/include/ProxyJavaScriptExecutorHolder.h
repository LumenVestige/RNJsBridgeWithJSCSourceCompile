//
// Created by sanyinchen on 2024/7/26.
//

#pragma once

#include <fbjni/fbjni.h>
#include "JavaScriptExecutorHolder.h"
#include "ProxyExecutor.h"

namespace facebook {
    namespace react {

        struct JavaJSExecutor : public jni::JavaClass<JavaJSExecutor> {
            static constexpr auto kJavaDescriptor = "Lcom/sanyinchen/jsbridge/executor/java/JavaJSExecutor;";
        };

        class ProxyJavaScriptExecutorHolder
                : public jni::HybridClass<ProxyJavaScriptExecutorHolder, JavaScriptExecutorHolder> {
        public:
            static constexpr auto kJavaDescriptor = "Lcom/sanyinchen/jsbridge/executor/ProxyJavaScriptExecutor;";

            static jni::local_ref<jhybriddata> initHybrid(
                    jni::alias_ref<jclass>, jni::alias_ref<JavaJSExecutor::javaobject> executorInstance) {
                return makeCxxInstance(
                        std::make_shared<ProxyExecutorOneTimeFactory>(
                                make_global(executorInstance)));
            }

            static void registerNatives() {
                registerHybrid({
                                       makeNativeMethod("initHybrid", ProxyJavaScriptExecutorHolder::initHybrid),
                               });
            }

        private:
            friend HybridBase;
            using HybridBase::HybridBase;
        };
    }
}
