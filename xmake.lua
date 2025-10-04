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

add_repositories("my-repo repo")

add_requires("litehtml", "pango", "libjpeg-turbo", "libwebp", "giflib", "aklomp-base64", "fmt")
set_languages("c++17")
add_requires("libavif", {configs = { aom = true }})
add_requires("cairo", {configs = { xlib = false }})
add_requireconfs("**.cairo", { override = true, configs = { xlib = false } })
add_requires("python", { system = true, version = "3.10.11", configs = { shared = true } })
add_requireconfs("**.python", { override = true, version = "3.10.11", configs = { headeronly = true, shared = true } })
add_requireconfs("**|python|cmake|ninja|meson", { override = true, system = false, configs = { shared = false } })
function require_htmlkit()
    if is_plat("linux") then
        if is_arch("x86_64") then
            add_linkorders("pangocairo-1.0", "pango-1.0")
            add_linkorders("pangoft2-1.0", "pango-1.0")
        else
            add_linkorders("pangocairo-1.0", "pangoft2-1.0", "pango-1.0")
        end
    end
    add_packages("litehtml", "cairo", "pango", "python", "libjpeg-turbo", "libwebp", "libavif", "giflib")
    add_packages("aklomp-base64", "fmt")
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
