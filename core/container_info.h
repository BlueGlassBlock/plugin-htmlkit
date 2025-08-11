#ifndef CONTAINER_INFO_H
#define CONTAINER_INFO_H

#include "cairo/cairo.h"
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
};

#endif // CONTAINER_INFO_H