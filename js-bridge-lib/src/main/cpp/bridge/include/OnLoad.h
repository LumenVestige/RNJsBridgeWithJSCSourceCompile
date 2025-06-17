// Copyright (c) Facebook, Inc. and its affiliates.

// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <jni.h>
#include "bridge/include/ReadableNativeMap.h"
#include "bridge/include/NativeArray.h"
#include "bridge/include/ReadableNativeArray.h"
#include "bridge/include/WritableNativeArray.h"
#include "bridge/include/WritableNativeMap.h"

namespace facebook {
    namespace react {

        jmethodID getLogMarkerMethod();

    } // namespace react
} // namespace facebook
