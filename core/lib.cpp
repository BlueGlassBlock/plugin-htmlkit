#define Py_LIMITED_API 0x03090000
#define PY_SSIZE_T_CLEAN
#include <chrono>
#include <utility>
#include <Python.h>
#include "container_info.h"
#include "htmlkit_container.h"
#include "fontconfig/fontconfig.h"
#include "litehtml.h"

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

extern "C" {

static PyObject* render(PyObject* mod, PyObject* args) {
	auto time_points = std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>>(0);
	auto record_now = [&time_points](std::string msg) {
		time_points.emplace_back(msg, std::chrono::high_resolution_clock::now());
	};

	record_now("start");

    FcConfig* fc = FcConfigCreate();
    if (FcConfigParseAndLoadFromMemory(fc, (FcChar8*) fc_config_windows, FcTrue) != FcTrue) {
	    PyErr_SetString(PyExc_RuntimeError, "Could not load fontconfig");
    	return nullptr;
    }
	record_now("fc_create");
	if (FcConfigSetCurrent(fc) != FcTrue) {
		PyErr_SetString(PyExc_RuntimeError, "Could not set current fontconfig");
		return nullptr;
	}
	record_now("fc_set");
    PyObject* log_fn = nullptr;
    const char *font_name, *lang, *culture, *html_content, *base_url;
    int width, height;
    container_info info;
    if (!PyArg_ParseTuple(args, "ssiiiisssO", &html_content, &base_url, &info.dpi, &width, &height,
                          &info.default_font_size_pt, &font_name, &lang, &culture, &log_fn)) {
        return nullptr;
    }
    info.width = width;
    info.height = height;
    info.default_font_name = std::string(font_name);
    info.language = std::string(lang);
    info.culture = std::string(culture);
    info.font_options = cairo_font_options_create();
    std::string html_content_str(html_content), base_url_str(base_url);
    cairo_font_options_set_antialias(info.font_options, CAIRO_ANTIALIAS_DEFAULT);
    cairo_font_options_set_hint_style(info.font_options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_subpixel_order(info.font_options, CAIRO_SUBPIXEL_ORDER_DEFAULT);

	record_now("font_option_create");

    htmlkit_container container(base_url_str, info, log_fn);

	record_now("container_create");

    auto doc = litehtml::document::createFromString(html_content_str, &container);

	record_now("document_create");

	doc->render(width);

	record_now("document_render");

    if (auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, doc->height())) {
        auto cr = cairo_create(surface);

        // Fill background with white color
        cairo_save(cr);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        cairo_fill(cr);
        cairo_restore(cr);

        // Draw document
        litehtml::position clip(0, 0, width, doc->height());


        doc->draw((litehtml::uint_ptr)cr, 0, 0, &clip);

        cairo_surface_flush(surface);
        cairo_destroy(cr);

    	record_now("complete_draw");

    	std::vector<unsigned char> bytes;
    	cairo_surface_write_to_png_stream(surface, cairo_wrapper::write_png_to_vec, &bytes);
        cairo_surface_destroy(surface);

    	record_now("write");

    	for (size_t i = 1; i < time_points.size(); ++i) {
    		auto first_msg = time_points[i - 1].first, second_msg = time_points[i].first;
    		auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(time_points[i].second - time_points[i - 1].second).count();
    		if (log_fn && PyCallable_Check(log_fn)) {
    			PyObject_CallFunction(log_fn, "s, s, s, i, s", "core.container.render_time", first_msg.c_str(), second_msg.c_str(), millisec, "ms");
    		}
    	}

        return PyBytes_FromStringAndSize((const char*)bytes.data(), bytes.size());
    }

    return nullptr;
}
}

static PyMethodDef methods[] = {
    {
        /* .ml_name = */ "render",
        /*.ml_meth = */render,
        /*.ml_flags = */METH_VARARGS,
        /*.ml_doc = */"Core function for rendering HTML page."
    },
    {
        nullptr, nullptr, 0, nullptr
    }
};

static struct PyModuleDef core_module = {
    /*.m_base = */PyModuleDef_HEAD_INIT,
    /*.m_name = */"core",
    /*.m_doc = */"Native core of htmlkit, built with litehtml.",
    /*.m_size = */0, // fontconfig is global
    /*.m_methods = */methods,
};

PyMODINIT_FUNC PyInit_core(void) {
    return PyModuleDef_Init(&core_module);
}
