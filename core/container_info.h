#ifndef CONTAINER_INFO_H
#define CONTAINER_INFO_H

#include "cairo/cairo.h"
#include <string>

struct container_info {
    int dpi;
    int width;
    int height;
    int default_font_size_pt;
    std::string default_font_name;
    // The "zh" part in "zh-CN"
    std::string language;
    // The "CN" part in "zh-CN"
    std::string culture;
    cairo_font_options_t* font_options;
};

#endif // CONTAINER_INFO_H