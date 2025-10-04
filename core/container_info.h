/*
Copyright (C) 2025 NoneBot

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONTAINER_INFO_H
#define CONTAINER_INFO_H

#include <cairo.h>
#include <string>

#include "litehtml/types.h"

struct container_info {
    litehtml::pixel_t dpi;
    litehtml::pixel_t width;
    litehtml::pixel_t height;
    litehtml::pixel_t default_font_size_pt;
    std::string default_font_name;
    // The "zh" part in "zh-CN"
    std::string language;
    // The "CN" part in "zh-CN"
    std::string culture;
    cairo_font_options_t* font_options;
    bool native_data_scheme;
};

#endif // CONTAINER_INFO_H