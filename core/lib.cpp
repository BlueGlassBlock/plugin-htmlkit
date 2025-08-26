#include <chrono>
#include <utility>
#include <thread>
#include <Python.h>
#include <fontconfig/fontconfig.h>
#include <litehtml.h>

#include "container_info.h"
#include "font_wrapper.h"
#include "htmlkit_container.h"

extern "C" {
    static PyObject* render(PyObject* mod, PyObject* args) {
        PyObject *exception_fn = nullptr, *asyncio_run_coroutine_threadsafe = nullptr, *urljoin = nullptr,
                 *asyncio_loop = nullptr, *img_fetch_fn = nullptr, *css_fetch_fn = nullptr;
        const char *font_name, *lang, *culture, *html_content, *base_url;
        int arg_dpi, arg_width, arg_height, default_font_size;
        container_info info;
        if (!PyArg_ParseTuple(args, "ssiiiisssOOOOOO", &html_content, &base_url, &arg_dpi, &arg_width, &arg_height,
                              &default_font_size, &font_name, &lang, &culture, &exception_fn,
                              &asyncio_run_coroutine_threadsafe, &urljoin, &asyncio_loop, &img_fetch_fn,
                              &css_fetch_fn)) {
            return nullptr;
        }
        info.dpi = arg_dpi;
        info.width = arg_width;
        info.height = arg_height;
        info.default_font_size_pt = default_font_size;
        info.default_font_name = std::string(font_name);
        info.language = std::string(lang);
        info.culture = std::string(culture);
        info.font_options = cairo_font_options_create();
        std::string html_content_str(html_content), base_url_str(base_url);
        cairo_font_options_set_antialias(info.font_options, CAIRO_ANTIALIAS_DEFAULT);
        cairo_font_options_set_hint_style(info.font_options, CAIRO_HINT_STYLE_NONE);
        cairo_font_options_set_subpixel_order(info.font_options, CAIRO_SUBPIXEL_ORDER_DEFAULT);

        if (asyncio_loop == nullptr) {
            PyErr_SetString(PyExc_TypeError, "Invalid asyncio event loop");
            return nullptr;
        }

        PyObject* future = PyObject_CallMethod(asyncio_loop, "create_future", nullptr);
        if (future == nullptr) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create future from event loop");
        }

        std::thread([=]() {
            auto bail = [=](const PyGILState_STATE gil) {
                PyObject *exc_ty, *exc_val, *exc_tb;
                PyErr_Fetch(&exc_ty, &exc_val, &exc_tb);
                if (PyObject_CallMethod(future, "set_exception", "O", exc_val) == nullptr) {
                    PyErr_Restore(exc_ty, exc_val, exc_tb);
                    PyErr_Print();
                }
                PyGILState_Release(gil);
            };

            auto time_points = std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>>(0);
            auto record_now = [&time_points](std::string msg) {
                printf("%s\n", msg.c_str());
                time_points.emplace_back(msg, std::chrono::high_resolution_clock::now());
            };

            record_now("start");

            PangoFontMap* font_map = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
            pango_cairo_font_map_set_default(PANGO_CAIRO_FONT_MAP(font_map));

            record_now("create_font_map");

            htmlkit_container container(base_url_str, info);
            container.urljoin = urljoin;
            container.asyncio_run_coroutine_threadsafe = asyncio_run_coroutine_threadsafe;
            container.m_loop = asyncio_loop;
            container.m_img_fetch_fn = img_fetch_fn;
            container.exception_logger = exception_fn;
            container.m_css_fetch_fn = css_fetch_fn;

            record_now("container_create");

            auto doc = litehtml::document::createFromString(html_content_str, &container, litehtml::master_css,
                                                            " html { background-color: #fff; }");

            record_now("document_create");

            int width = arg_width;
            litehtml::pixel_t best_width = doc->render(arg_width);
            if (best_width < width) {
                width = best_width;
                doc->render(width);
            }

            record_now("document_render");

            auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, doc->content_height());
            if (!surface) {
                const auto gil = PyGILState_Ensure();
                PyErr_SetString(PyExc_RuntimeError, "Could not create surface");
                return bail(gil);
            }
            auto cr = cairo_create(surface);

            // Fill background with white color
            cairo_save(cr);
            cairo_rectangle(cr, 0, 0, width, doc->content_height());
            cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
            cairo_fill(cr);
            cairo_restore(cr);

            // Draw document
            litehtml::position clip(0, 0, width, doc->content_height());

            doc->draw((litehtml::uint_ptr)cr, 0, 0, &clip);

            cairo_surface_flush(surface);
            cairo_destroy(cr);

            record_now("complete_draw");

            std::vector<unsigned char> bytes;
            cairo_surface_write_to_png_stream(surface, cairo_wrapper::write_to_vector, &bytes);

            const auto gil = PyGILState_Ensure();
            PyObject* bytes_obj = PyBytes_FromStringAndSize((const char*)bytes.data(), bytes.size());
            if (bytes_obj == nullptr) {
                return bail(gil);
            }
            PyObject* set_result = PyObject_GetAttrString(future, "set_result");
            if (set_result == nullptr) {
                Py_DECREF(bytes_obj);
                return bail(gil);
            }
            PyObject* call_soon_result = PyObject_CallMethod(asyncio_loop, "call_soon_threadsafe", "OO", set_result,
                                                             bytes_obj);
            if (call_soon_result == nullptr) {
                Py_DECREF(bytes_obj);
                Py_DECREF(set_result);
                return bail(gil);
            }
            Py_DECREF(bytes_obj);
            Py_DECREF(set_result);
            Py_DECREF(call_soon_result);
            PyGILState_Release(gil);

            record_now("write_to_stream");

            cairo_surface_destroy(surface);

            for (size_t i = 1; i < time_points.size(); ++i) {
                auto first_msg = time_points[i - 1].first, second_msg = time_points[i].first;
                auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(
                    time_points[i].second - time_points[i - 1].second).count();
                printf("core.container.render_time %s -> %s: %lld ms\n", first_msg.c_str(), second_msg.c_str(),
                       millisec);
            }
            return;
        }).detach();
        return future;
    }

    static PyObject* setup_fontconfig(PyObject* mod, PyObject* args) {
        init_fontconfig();
        Py_RETURN_NONE;
    }

    static PyMethodDef methods[] = {
        {
            /* .ml_name = */ "render",
            /*.ml_meth = */render,
            /*.ml_flags = */METH_VARARGS,
            /*.ml_doc = */"Core function for rendering HTML page."
        },
        {
            /* .ml_name = */ "setup_fontconfig",
            /*.ml_meth = */setup_fontconfig,
            /*.ml_flags = */METH_VARARGS,
            /*.ml_doc = */"Setup fontconfig if not already initialized."
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
}
