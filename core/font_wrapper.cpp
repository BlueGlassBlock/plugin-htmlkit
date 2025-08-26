#include "font_wrapper.h"

#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>
#include <shared_mutex>

static PangoFontMap* global_fontmap = nullptr;
static std::shared_mutex global_fontmap_mutex;

static const char* fc_config_windows = R"(<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
	<cachedir>C:/Users/blueg/AppData/Local/Temp/htmlkit_fontconfig</cachedir>
	<dir>C:/Windows/Fonts</dir>
	<alias>
		<family>serif</family>
		<prefer>
			<family>Noto Serif</family>
		</prefer>
	</alias>
	<alias>
		<family>sans-serif</family>
		<prefer>
			<family>Noto Sans</family>
		</prefer>
	</alias>
	<alias>
		<family>fantasy</family>
		<prefer>
			<family>Noto Sans</family>
		</prefer>
	</alias>
	<alias>
		<family>cursive</family>
		<prefer>
			<family>Noto Sans</family>
		</prefer>
	</alias>
	<alias>
		<family>monospace</family>
		<prefer>
			<family>Noto Sans Mono</family>
		</prefer>
	</alias>
</fontconfig>
)";

int init_fontconfig() {
    FcConfig* cfg = FcInitLoadConfigAndFonts();
    if (!cfg) {
        cfg = FcConfigCreate();
        if (!cfg) {
            return -1;
        }
        if (FcConfigParseAndLoadFromMemory(cfg, (FcChar8*)fc_config_windows, FcTrue) != FcTrue) {
            FcConfigDestroy(cfg);
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
    printf("OK init success %p\n", global_fontmap);
    return 0;
}
