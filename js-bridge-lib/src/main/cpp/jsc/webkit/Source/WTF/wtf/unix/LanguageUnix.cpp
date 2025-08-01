/*
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2016, 2017 Apple Inc. All rights reserved.
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
 */

#include "config.h"
#include <wtf/Language.h>

#include <locale.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>
#include <sys/system_properties.h>

namespace WTF {

static String platformLanguage()
{
    char buf[2 * (PROP_VALUE_MAX + 1)];

    int locale_len = __system_property_get("persist.sys.locale", buf);
    if (locale_len == 0) {
      int lang_len = __system_property_get("persist.sys.language", buf);
      if(lang_len == 0) {
        return "en-US"_s;
      }
      int country_len = __system_property_get("persist.sys.country", buf + lang_len + 1);
      if(country_len != 0) {
        buf[lang_len] = '-';
      }
    }

    return String(buf);
}

Vector<String> platformUserPreferredLanguages()
{
    return { platformLanguage() };
}

} // namespace WTF
