#pragma once
#include "gdiplus_container.h"
#include <windows.h>

class dull_gdiplus_container : public gdiplus_container {
    std::string m_base_url;

  public:
    dull_gdiplus_container();
    ~dull_gdiplus_container();

  protected:
    void get_client_rect(litehtml::position &client) const override {
        client.x = 0;
        client.y = 0;
        client.width = 1920;
        client.height = 1080;
    }
    void set_caption(const char *caption) override {};
    void set_base_url(const char *base_url) override {
        m_base_url = base_url ? base_url : "";
    };
    void make_url(LPCWSTR url, LPCWSTR basepath, std::wstring &out) override;
    uint_ptr get_image(LPCWSTR url_or_path, bool redraw_on_ready) override;
    void on_anchor_click(const char *url, const litehtml::element::ptr &el) {}
    bool on_element_click(const litehtml::element::ptr & /*el*/) override {
        return false;
    }
    void on_mouse_event(const litehtml::element::ptr &el,
                        litehtml::mouse_event event) override {}
    void set_cursor(const char *cursor) override {}
    std::function<void()> import_css(const litehtml::string& url, const litehtml::string& baseurl, const std::function<void(const litehtml::string& css_text, const litehtml::string& new_baseurl)>& on_imported) override {
        return [&]() { on_imported(litehtml::string(), litehtml::string()); };
    };
    void get_viewport(litehtml::position &viewport) const override {}
};
