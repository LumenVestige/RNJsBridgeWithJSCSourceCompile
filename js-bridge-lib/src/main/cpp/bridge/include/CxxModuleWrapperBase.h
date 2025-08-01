// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <memory>
#include <string>

#include <CxxModule.h>
#include <fbjni/fbjni.h>

namespace facebook {
namespace react {

struct JNativeModule : jni::JavaClass<JNativeModule> {
  constexpr static const char *const kJavaDescriptor =
    "Lcom/sanyinchen/jsbridge/module/bridge/NativeModule;";
};

/**
 * The C++ part of a CxxModuleWrapper is not a unique class, but it
 * must extend this base class.
 */
class CxxModuleWrapperBase
  : public jni::HybridClass<CxxModuleWrapperBase, JNativeModule> {
public:
  constexpr static const char *const kJavaDescriptor =
    "Lcom/sanyinchen/jsbridge/module/impl/cxx/CxxModuleWrapperBase;";

  static void registerNatives() {
    registerHybrid({
      makeNativeMethod("getName", CxxModuleWrapperBase::getName)
    });
  }

  // JNI method
  virtual std::string getName() = 0;

  // Called by ModuleRegistryBuilder
  virtual std::unique_ptr<xplat::module::CxxModule> getModule() = 0;
};

}
}
