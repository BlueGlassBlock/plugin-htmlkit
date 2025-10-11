package("litehtml")
    set_homepage("http://www.litehtml.com/")
    set_description("Fast and lightweight HTML/CSS rendering engine")
    set_license("BSD-3-Clause")

    set_sourcedir(path.join(os.scriptdir(), "..", "..", "..", "..", "litehtml"))

    add_deps("cmake")
    add_deps("gumbo-parser")

    on_install(function (package)
        local configs = {"-DBUILD_TESTING=OFF", "-DLITEHTML_BUILD_TESTING=OFF", "-DEXTERNAL_GUMBO=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"}
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
