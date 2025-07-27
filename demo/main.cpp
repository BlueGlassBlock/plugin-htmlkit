#include "dull_gdiplus_container.h"
#include "windows_container.h"
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

void render_cairo(std::string file) {
    auto doc = litehtml::document::createFromString(
        file, new windows_container(), litehtml::master_css,
        "html { background-color: #fff; }");

    auto start = std::chrono::high_resolution_clock::now();
    int best_width = doc->render(1080);
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

void render_gdiplus(std::string file) {
    auto doc = litehtml::document::createFromString(
        file, new dull_gdiplus_container(), litehtml::master_css,
        "html { background-color: #fff; }");

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    auto start = std::chrono::high_resolution_clock::now();
    int best_width = doc->render(1080);
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

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <html_file>" << std::endl;
        return 1;
    }
    std::ifstream file;
    file.open(argv[1], std::ios::in);
    std::stringstream buffer;
    buffer << file.rdbuf();
    render_cairo(buffer.str());
    render_gdiplus(buffer.str());
    return 0;
}
