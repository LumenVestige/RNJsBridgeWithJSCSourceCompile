// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <string>

#include <fbjni/fbjni.h>
#include <folly/Memory.h>

#include "CxxModuleWrapper.h"
#include "JavaModuleWrapper.h"
#include "JMessageQueueThread.h"
#include "JSLoader.h"
#include "ModuleRegistryBuilder.h"
#include "ModuleRegistry.h"

namespace facebook {
    namespace react {

        class Instance;

        class JavaScriptExecutorHolder;

        class NativeArray;

        struct ReactCallback : public jni::JavaClass<ReactCallback> {
            static constexpr auto kJavaDescriptor = "Lcom/sanyinchen/jsbridge/common/callback/JsBridgeCallback;";
        };

        class JsBridgeInstanceImpl : public jni::HybridClass<JsBridgeInstanceImpl> {
        public:

            static constexpr auto kJavaDescriptor = "Lcom/sanyinchen/jsbridge/JsBridgeInstanceImpl;";

            static jni::local_ref<jhybriddata> initHybrid(jni::alias_ref<jclass>);

            ~JsBridgeInstanceImpl() override;

            static void registerNatives();

            std::shared_ptr<Instance> getInstance() {
                return instance_;
            }

        private:
            friend HybridBase;

            JsBridgeInstanceImpl();

            void initializeBridge(
                    jni::alias_ref<ReactCallback::javaobject> callback,
                    // This executor is actually a factory holder.
                    JavaScriptExecutorHolder *jseh,
                    jni::alias_ref<JavaMessageQueueThread::javaobject> jsQueue,
                    jni::alias_ref<JavaMessageQueueThread::javaobject> moduleQueue,
                    jni::alias_ref<jni::JCollection<JavaModuleWrapper::javaobject>::javaobject> javaModules,
                    jni::alias_ref<jni::JCollection<ModuleHolder::javaobject>::javaobject> cxxModules);

            void extendNativeModules(
                    jni::alias_ref<jni::JCollection<JavaModuleWrapper::javaobject>::javaobject> javaModules,
                    jni::alias_ref<jni::JCollection<ModuleHolder::javaobject>::javaobject> cxxModules);

            /**
             * Sets the source URL of the underlying bridge without loading any JS code.
             */
            void jniSetSourceURL(const std::string &sourceURL);

            /**
             * Registers the file path of an additional JS segment by its ID.
             *
             */
            void jniRegisterSegment(int segmentId, const std::string &path);

            void
            jniLoadScriptFromAssets(jni::alias_ref<JAssetManager::javaobject> assetManager, const std::string &assetURL,
                                    bool loadSynchronously);

            void
            jniLoadScriptFromFile(const std::string &fileName, const std::string &sourceURL, bool loadSynchronously);


            void jniCallJSFunction(std::string module, std::string method, NativeArray *arguments);

            void jniCallJSCallback(jint callbackId, NativeArray *arguments);

            void setGlobalVariable(std::string propName,
                                   std::string &&jsonValue);

            jlong getJavaScriptContext();

            void handleMemoryPressure(int pressureLevel);

            // This should be the only long-lived strong reference, but every C++ class
            // will have a weak reference.
            std::shared_ptr<Instance> instance_;
            std::shared_ptr<ModuleRegistry> moduleRegistry_;
            std::shared_ptr<JMessageQueueThread> moduleMessageQueue_;
        };

    }
}
