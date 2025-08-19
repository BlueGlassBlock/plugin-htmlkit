#include "dull_gdiplus_container.h"
#include "windows_container.h"
#include "htmlkit_container.h"
#include "fontconfig/fontconfig.h"
#include <gdiplus.h>
#include <windows.h>
#pragma comment(lib, "gdiplus.lib")
#include <cairo.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <litehtml.h>
#include <sstream>

int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
    UINT num = 0;  // number of image encoders
    UINT size = 0; // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo *pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1; // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1; // Failure

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j; // Success
        }
    }

    free(pImageCodecInfo);
    return -1; // Failure
}

void render_cairo(std::string file, int width) {
    auto doc = litehtml::document::createFromString(
        file, new windows_container(), litehtml::master_css,
        "html { background-color: #fff; }");

    auto start = std::chrono::high_resolution_clock::now();
    int best_width = width == -1 ? doc->render(1080) : width;
    doc->render(best_width);
    cairo_t *cr = cairo_create(
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, best_width, doc->content_height()));
    litehtml::position pos(0, 0, best_width, doc->content_height());

    doc->draw((litehtml::uint_ptr)cr, 0, 0, &pos);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    // Save the rendered document to a PNG file
    cairo_surface_t *surface = cairo_get_target(cr);
    cairo_surface_write_to_png(surface, "output_cairo.png");
    cairo_destroy(cr);
    std::cout << "Rendered document to output_cairo.png" << std::endl;
    std::cout << "Rendering took " << duration << " ms" << std::endl;
}

void render_gdiplus(std::string file, int width) {
    auto doc = litehtml::document::createFromString(
        file, new dull_gdiplus_container(), litehtml::master_css,
        "html { background-color: #fff; }");

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    auto start = std::chrono::high_resolution_clock::now();
    int best_width = width == -1 ? doc->render(1080) : width;
    doc->render(best_width);
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap =
        CreateCompatibleBitmap(hdcScreen, best_width, doc->content_height());
    SelectObject(hdcMem, hBitmap);
    ReleaseDC(NULL, hdcScreen);

    litehtml::position pos(0, 0, best_width, doc->content_height());
    doc->draw((litehtml::uint_ptr)hdcMem, 0, 0, &pos);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    Gdiplus::Bitmap bitmap(hBitmap, NULL);
    CLSID pngClsid;
    GetEncoderClsid(L"image/png", &pngClsid);
    bitmap.Save(L"output_gdiplus.png", &pngClsid, NULL);

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    std::cout << "Rendered document to output_gdiplus.png" << std::endl;
    std::cout << "Rendering took " << duration << " ms" << std::endl;
}


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

void render_htmlkit(std::string file, int width) {
	FcConfig* fc = FcConfigCreate();
	if (FcConfigParseAndLoadFromMemory(fc, (FcChar8*) fc_config_windows, FcTrue) != FcTrue) {
		return;
	}
	if (FcConfigSetCurrent(fc) != FcTrue) {
		return;
	}
	container_info info;
	info.dpi = 96.0;
	info.default_font_size_pt = 12.0;
	info.width = 1920;
	info.height = 1080;
	info.default_font_name = "sans-serif";
	info.language = "zh";
	info.culture = "CN";
	info.font_options = cairo_font_options_create();
	cairo_font_options_set_antialias(info.font_options, CAIRO_ANTIALIAS_DEFAULT);
	cairo_font_options_set_hint_style(info.font_options, CAIRO_HINT_STYLE_NONE);
	cairo_font_options_set_subpixel_order(info.font_options, CAIRO_SUBPIXEL_ORDER_DEFAULT);

	htmlkit_container container("", info);

	auto doc = litehtml::document::createFromString(
		file, &container, litehtml::master_css, "html {background-color: #fff;}");

	auto start = std::chrono::high_resolution_clock::now();
	int best_width = width == -1 ? doc->render(1920) : width;
	doc->render(best_width);
	cairo_t *cr = cairo_create(
		cairo_image_surface_create(CAIRO_FORMAT_ARGB32, best_width, doc->content_height()));
	litehtml::position pos(0, 0, best_width, doc->content_height());

	doc->draw((litehtml::uint_ptr)cr, 0, 0, &pos);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration =
		std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
			.count();

	// Save the rendered document to a PNG file
	cairo_surface_t *surface = cairo_get_target(cr);
	cairo_surface_write_to_png(surface, "output_htmlkit.png");
	cairo_destroy(cr);
	std::cout << "Rendered document to output_htmlkit.png" << std::endl;
	std::cout << "Rendering took " << duration << " ms" << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <html_file> [width]" << std::endl;
        return 1;
    }
    std::ifstream file;
    file.open(argv[1], std::ios::in);
    std::stringstream buffer;
    buffer << file.rdbuf();
    int width = -1;
    if (argc > 2) {
        width = std::stoi(argv[2]);
        std::cout << "Using width: " << width << std::endl;
    }
    render_cairo(buffer.str(), width);
    render_gdiplus(buffer.str(), width);
	render_htmlkit(buffer.str(), width);
    return 0;
}
