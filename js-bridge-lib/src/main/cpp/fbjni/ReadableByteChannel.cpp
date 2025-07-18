/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ReadableByteChannel.h"

namespace facebook {
namespace jni {

int JReadableByteChannel::read(alias_ref<JByteBuffer> dest) const {
  if (!self()) {
    throwNewJavaException(
        "java/lang/NullPointerException", "java.lang.NullPointerException");
  }
  static auto method =
      javaClassStatic()->getMethod<jint(alias_ref<JByteBuffer>)>("read");
  return method(self(), dest);
}

} // namespace jni
} // namespace facebook
