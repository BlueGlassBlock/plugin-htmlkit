# nonebot-plugin-htmlkit

一个基于 [litehtml](https://github.com/litehtml/litehtml) 的轻量级 HTML 渲染插件。

## 特性

- 基于 [fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/) 的字体管理, 支持系统字体和自定义字体
- 提供了 HTML，纯文本，markdown，和 Jinja2 模板渲染的快捷函数
- 支持自定义图片和 CSS 的加载策略
- 支持通过 CSS 控制样式
- 支持自适应控制渲染宽度

## 安装

使用 [`nb-cli`](https://cli.nonebot.dev/) 安装：

```bash
nb plugin install nonebot-plugin-htmlkit
```

或者，使用你选择的 Python 包管理器工具安装 `nonebot-plugin-htmlkit` 即可。

## 使用

### API

```python
from nonebot import require

require("nonebot_plugin_htmlkit")
from nonebot_plugin_htmlkit import (
    text_to_pic,
    md_to_pic,
    template_to_pic,
    html_to_pic,
)
```

> [!CAUTION]
> 注意：请先 `require("nonebot_plugin_htmlkit")` 后再 `import` 插件！！！

### 配置项

`plugin-htmlkit` 的配置项主要为 [fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/) 的相关配置。

对于 `FC/FONTCONFIG` 开头的配置项，请参考 [fontconfig 文档](https://www.freedesktop.org/software/fontconfig/fontconfig-user.html) 以了解更多。

```python
# ===============================
# Fontconfig 配置
# ===============================

# FONTCONFIG_FILE
# 用于覆盖默认的配置文件路径。
FONTCONFIG_FILE: str

# FONTCONFIG_PATH
# 用于覆盖默认的配置目录。
FONTCONFIG_PATH: str

# FONTCONFIG_SYSROOT
# 用于设置默认的 sysroot 目录。
FONTCONFIG_SYSROOT: str

# FC_DEBUG
# 用于输出详细的调试信息。
# 详细见 fontconfig 文档。
FC_DEBUG: str

# FC_DBG_MATCH_FILTER
# 用于在调试时过滤特定模式。
# 仅当 FC_DEBUG 设置为 MATCH2 时生效。
FC_DBG_MATCH_FILTER: str

# FC_LANG
# 用于指定查询时的默认语言（弱绑定）。
# 如果未设置，则从当前 locale 推导。
FC_LANG: str

# FONTCONFIG_USE_MMAP
# 控制是否使用 mmap(2) 来处理缓存文件（如果可用）。
# 值为布尔类型（yes/no, 1/0）。
# 如果显式设置该变量，将跳过系统检查并强制启用或禁用。
FONTCONFIG_USE_MMAP: str
```

### 构建说明

1. [安装 Xmake](https://xmake.io/zh/guide/quick-start#installation)
1. 初始化环境

   使用 Xmake 时必须激活 Python 虚拟环境，并且安装 `build` 依赖组。

   ```bash
   # 拉取子模块
   git submodule update --init --recursive
   # 创建虚拟环境并安装依赖，同时避免直接构建项目
   uv sync --no-install-workspace
   # 激活虚拟环境，请使用对应 shell 的命令
   source .venv/bin/activate
   # 配置 Xmake 项目并安装依赖（由于有大量依赖需要通过源码编译安装，可能耗时较长）
   xmake config -m releasedbg
   ```

1. 构建并安装

   ```bash
   # 构建
   xmake build
   # 安装
   xmake install
   # 安装到当前虚拟环境
   uv sync --reinstall-package nonebot-plugin-htmlkit
   ```

   如果对 [litehtml](./litehtml) 做了修改，则需要重新构建它：

   ```bash
   xmake require --force litehtml
   # 或者用以下更 dirty 但是快速的方法
   rm -r build
   xmake clean --all
   # 重新构建并安装
   xmake build
   xmake install
   uv sync --reinstall-package nonebot-plugin-htmlkit
   ```

#### 许可证

本插件的 [Python 部分](./nonebot_plugin_htmlkit) 在 MIT 许可证下发布，[C++ 部分](./core) 在 LGPL-3.0-or-later 许可证下发布。
