import("core.base.json")
import("lib.detect.find_path")
import("lib.detect.find_library")
import("lib.detect.find_program")

function _find_binary(package, opt)
    local result = package:find_tool("python3", opt)
    if not result then
        result = package:find_tool("python", opt)
    end
    if result then
        -- check if pip, setuptools and wheel are installed
        local ok = try { function()
            os.vrunv(result.program, { "-c", "import pip" })
            os.vrunv(result.program, { "-c", "import setuptools" })
            os.vrunv(result.program, { "-c", "import wheel" })
            return true
        end }
        if not ok then
            return false
        end
    end
end

local _find_library_py = [[
import json
import sys
from sysconfig import get_config_var

print(
    json.dumps(
        {
            "version": sys.version.split()[0],
            "base_prefix": sys.base_prefix,
            "gil_disabled": bool(get_config_var("Py_GIL_DISABLED")),
            "debug": bool(get_config_var("Py_DEBUG")),
        }
    )
)
]]

function _find_library(package, opt)
    -- init search options
    opt = opt or {}
    opt.paths = opt.paths or {}
    table.insert(opt.paths, "$(env PATH)")
    table.insert(opt.paths, "$(env CONDA_PREFIX)")

    -- find python
    local program = find_program("python3", opt)
    if not program then
        program = find_program("python", opt)
    end
    if not program then
        return false
    end

    local out = try { function () return json.decode(os.iorunv(program, { "-c", _find_library_py })) end }
    if not out then
        return false
    end

    local version = out.version
    if out.gil_disabled then
        version = version .. "t"
    end
    if out.debug then
        version = version .. "d"
    end

    -- find library and header
    local exepath = path.directory(program)
    local link = nil
    local libpath = nil
    local includepath = nil
    if package:is_plat("windows") then
        link = "python" .. table.concat(table.slice(version:split("%."), 1, 2), "")
        if out.gil_disabled then
            link = link .. "t"
        end
        if out.debug then
            link = link .. "_d"
        end
        libpath = find_library(link, { exepath, out.base_prefix }, { suffixes = { "libs" } })
        includepath = find_path("Python.h", { exepath, out.base_prefix }, { suffixes = { "include" } })
    else
        link = "python" .. table.concat(table.slice(version:split("%."), 1, 2), ".")
        if out.gil_disabled then
            link = link .. "t"
        end
        if out.debug then
            link = link .. "d"
        end
        libpath = find_library(link, { path.directory(exepath), out.base_prefix }, { suffixes = { "lib", "lib64", "lib/x86_64-linux-gnu" } })
        includepath = find_path("Python.h", { path.directory(exepath), out.base_prefix }, { suffixes = { "include/" .. link } })
    end

    if not includepath then
        return
    end
    local result = {
        version = version,
        includedirs = includepath
    }

    if package:config("headeronly") then
        return result
    end
    if libpath then
        result.links = libpath.link
        result.linkdirs = libpath.linkdir
        return result
    end
end

function main(package, opt)
    if opt.system then
        if package:is_binary() then
            return _find_binary(package, opt)
        else
            return _find_library(package, opt)
        end
    end
end
