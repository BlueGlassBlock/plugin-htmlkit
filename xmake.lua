add_rules("mode.debug", "mode.release", "mode.releasedbg")

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

add_requires("litehtml_local", "python 3.9.x")
set_languages("c++17")
add_requires("pango", {configs = {shared = true}})
add_requires("cairo", {configs = {shared = true}})
add_requires("fontconfig", {configs = {shared = true}})

add_requireconfs("**.python", {override = true, version = "3.9.x", headeronly = true })

function require_htmlkit()
    add_packages("litehtml_local", "cairo", "pango", "python")
    add_files("core/*.cpp")
    add_defines("UNICODE", "PY_SSIZE_T_CLEAN")
    if is_plat("windows") then
        add_links("Dwrite")
    end
end

target("core")
    set_kind("shared")
    set_prefixname("")
    set_prefixdir("/", {bindir = ".", libdir = ".", includedir = "."})
    set_extension(".dylib")
    require_htmlkit()
