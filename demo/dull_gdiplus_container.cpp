#include "dull_gdiplus_container.h"

#include <windows.h>

dull_gdiplus_container::dull_gdiplus_container() {}
dull_gdiplus_container::~dull_gdiplus_container() {}

void dull_gdiplus_container::make_url(LPCWSTR url, LPCWSTR basepath,
                                      std::wstring &out) {
    if (!url || !*url) {
        out.clear();
        return;
    }

    // If the URL is already absolute (starts with protocol), return it as-is
    if (wcsstr(url, L"://") != nullptr) {
        out = url;
        return;
    }

    // If no basepath provided, just return the url
    if (!basepath || !*basepath) {
        out = url;
        return;
    }

    std::wstring base(basepath);

    // Remove trailing slash from basepath if present
    if (!base.empty() && (base.back() == L'/' || base.back() == L'\\')) {
        base.pop_back();
    }

    // Remove leading slash from url if present
    LPCWSTR urlPtr = url;
    while (*urlPtr == L'/' || *urlPtr == L'\\') {
        urlPtr++;
    }

    // Combine basepath and url
    out = base + L"/" + urlPtr;
}

litehtml::uint_ptr dull_gdiplus_container::get_image(LPCWSTR url_or_path,
                                                     bool redraw_on_ready) {
    return 0;
}
