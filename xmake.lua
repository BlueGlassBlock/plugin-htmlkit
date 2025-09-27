--[[
Copyright (C) 2025 NoneBot

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <https://www.gnu.org/licenses/>.
]]
add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_license("LGPL-3.0-or-later")
package("litehtml_local")
    set_homepage("http://www.litehtml.com/")
    set_description("Fast and lightweight HTML/CSS rendering engine")
    set_license("BSD-3-Clause")

    set_sourcedir(path.join(os.scriptdir(), "litehtml"))

    add_deps("cmake")
    add_deps("gumbo-parser")

    on_install(function (package)
        local configs = {"-DBUILD_TESTING=OFF", "-DLITEHTML_BUILD_TESTING=OFF", "-DEXTERNAL_GUMBO=ON"}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        if package:is_plat("windows") and package:config("shared") then
            table.insert(configs, "-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON")
        end
        import("package.tools.cmake").install(package, configs, {packagedeps = "gumbo-parser"})
        os.cp("include/litehtml.h", package:installdir("include"))
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <string>
            #include <litehtml.h>
            using namespace litehtml;
            void test() {
                css_selector selector;
                selector.parse(".class", no_quirks_mode);
            }
        ]]}, {configs = {languages = "c++17"}}))
    end)
package_end()

add_requires("litehtml_local", "pango", "cairo", "libjpeg-turbo", "aklomp-base64", "fmt")
set_languages("c++17")
add_requires("python", { system = true, version = "3.10.11", configs = { shared = true } })
add_requireconfs("**.python", { override = true, version = "3.10.11", headeronly = true, shared = true })
add_requireconfs("**|python|cmake|ninja|meson", { override = true, system = false, shared = false })
function require_htmlkit()
    if is_plat("linux") then
        if is_arch("x86_64") then
            add_linkorders("pangocairo-1.0", "pango-1.0")
            add_linkorders("pangoft2-1.0", "pango-1.0")
        else
            add_linkorders("pangocairo-1.0", "pangoft2-1.0", "pango-1.0")
        end
    end
    add_packages("litehtml_local", "cairo", "cairo_jpg", "pango", "python", "libjpeg-turbo", "aklomp-base64", "fmt")
    add_packages("python", { links = {} })
    add_files("core/*.cpp")
    add_defines("UNICODE", "PY_SSIZE_T_CLEAN", "Py_LIMITED_API=0x030a0000")  -- Python 3.10
    if is_plat("windows") then
        add_links("Dwrite")
    end
    if is_plat("macosx") then
        -- Pango CoreText backend needs CoreText (and CoreGraphics/CoreFoundation for related symbols)
        add_frameworks("CoreText", "CoreGraphics", "CoreFoundation")
        add_ldflags("-undefined", "dynamic_lookup", {force = true})
        add_shflags("-undefined", "dynamic_lookup", {force = true})
    end
end

target("core")
    set_kind("shared")
    set_prefixname("")
    set_prefixdir("/", {bindir = ".", libdir = ".", includedir = "."})
    set_extension(".dylib")
    require_htmlkit()
