#include "font_wrapper.h"

#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <shared_mutex>

static PangoFontMap* global_fontmap = nullptr;
static std::shared_mutex global_fontmap_mutex;

int init_fontconfig() {
    FcConfig* cfg = FcInitLoadConfigAndFonts();
    if (!cfg) {
        cfg = FcConfigCreate();
        if (!cfg) {
            return -1;
        }
    }
    if (FcConfigSetCurrent(cfg) != FcTrue) {
        FcConfigDestroy(cfg);
        return -1;
    }
    global_fontmap_mutex.lock();
    if (global_fontmap != nullptr) {
        g_object_unref(global_fontmap);
    }
    global_fontmap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
    g_object_ref_sink(global_fontmap);
    global_fontmap_mutex.unlock();
    return 0;
}
