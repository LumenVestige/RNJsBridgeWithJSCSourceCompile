/*
 * Copyright (C) 2006-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#if defined(HAVE_CONFIG_H) && HAVE_CONFIG_H && defined(BUILDING_WITH_CMAKE)
#include "cmakeconfig.h"
#endif

#define JSC_API_AVAILABLE(...)
#define JSC_CLASS_AVAILABLE(...) JS_EXPORT
#define JSC_API_DEPRECATED(...)
// Use zero since it will be less than any possible version number.
#define JSC_MAC_VERSION_TBA 0
#define JSC_IOS_VERSION_TBA 0

#include "runtime/JSExportMacros.h"

#ifdef __cplusplus
#undef new
#undef delete
#include <wtf/FastMalloc.h>
#endif

#include <wtf/DisallowCType.h>
