// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <string>

#include <CxxModule.h>
#include <ModuleRegistry.h>
#include <fbjni/fbjni.h>

#include "CxxModuleWrapper.h"
#include "JavaModuleWrapper.h"

namespace facebook {
namespace react {

class MessageQueueThread;

class ModuleHolder : public jni::JavaClass<ModuleHolder> {
 public:
  static auto constexpr kJavaDescriptor =
    "Lcom/sanyinchen/jsbridge/module/bridge/NativeModuleHolder;";

  std::string getName() const;
  xplat::module::CxxModule::Provider getProvider() const;
};

std::vector<std::unique_ptr<NativeModule>> buildNativeModuleList(
  std::weak_ptr<Instance> winstance,
  jni::alias_ref<jni::JCollection<JavaModuleWrapper::javaobject>::javaobject> javaModules,
  jni::alias_ref<jni::JCollection<ModuleHolder::javaobject>::javaobject> cxxModules,
  std::shared_ptr<MessageQueueThread> moduleMessageQueue);
}
}
