/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "detail/Log.h"

void fb_printLog(int prio, const char *tag, const char *fmt, ...) {
    FBJNI_LOGF(tag);
}
