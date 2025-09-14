from asyncio import get_running_loop, run_coroutine_threadsafe
from collections.abc import Callable, Coroutine
import os
from pathlib import Path
from typing import Any
from urllib.parse import urljoin

import aiofiles
import jinja2
import markdown

import nonebot
from nonebot import get_driver
from nonebot.drivers import HTTPClientMixin, Request
from nonebot.log import logger
from nonebot.plugin import PluginMetadata, get_plugin_config

from . import config, core
from .config import FcConfig

__plugin_meta__ = PluginMetadata(
    name="nonebot-plugin-htmlkit",
    description="轻量级的 HTML 渲染工具",
    usage="",
    type="library",
    homepage="https://github.com/nonebot/plugin-htmlkit",
    extra={},
)

driver = nonebot.get_driver()


@driver.on_startup
def init_fontconfig(**kwargs: Any):
    logger.info("Initializing fontconfig...")
    with config.set_fc_environ(get_plugin_config(FcConfig)):
        core._init_fontconfig_internal()  # pyright: ignore[reportPrivateUsage]
    logger.info("Fontconfig initialized.")


ImgFetchFn = Callable[[str], Coroutine[Any, Any, bytes | None]]
CSSFetchFn = Callable[[str], Coroutine[Any, Any, str | None]]


async def none_fetcher(_url: str) -> None:
    return None


async def read_file(path: str) -> str:
    async with aiofiles.open(path, encoding="UTF8") as f:
        return await f.read()


async def read_tpl(path: str) -> str:
    return await read_file(f"{TEMPLATES_PATH}/{path}")


async def filesystem_img_fetcher(url: str) -> bytes | None:
    if url.startswith("file://"):
        path = url[7:]
        if os.path.isfile(path):
            try:
                async with aiofiles.open(path, "rb") as f:
                    return await f.read()
            except Exception as e:
                logger.opt(exception=e).warning(f"Failed to read local file {path}")
    return None


async def network_img_fetcher(url: str) -> bytes | None:
    driver = get_driver()
    if not isinstance(driver, HTTPClientMixin):
        logger.critical(
            "Driver does not support HTTP requests. "
            "Please initialize NoneBot with HTTP client drivers like HTTPX or AIOHTTP."
        )
        return None
    try:
        response = await driver.request(Request("GET", url))
        if isinstance(response.content, bytes):
            return response.content
        return None
    except Exception as e:
        logger.opt(exception=e).warning(f"Failed to fetch resource from {url}")
    return None


async def combined_img_fetcher(url: str) -> bytes | None:
    content = await filesystem_img_fetcher(url)
    if content is not None:
        return content
    return await network_img_fetcher(url)


async def filesystem_css_fetcher(url: str) -> str | None:
    if url.startswith("file://"):
        path = url[7:]
        if os.path.isfile(path):
            try:
                async with aiofiles.open(path, encoding="utf-8") as f:
                    return await f.read()
            except Exception as e:
                logger.opt(exception=e).warning(f"Failed to read local CSS file {path}")
    return None


async def network_css_fetcher(url: str) -> str | None:
    driver = get_driver()
    if not isinstance(driver, HTTPClientMixin):
        logger.critical(
            "Driver does not support HTTP requests. "
            "Please initialize NoneBot with HTTP client drivers like HTTPX or AIOHTTP."
        )
        return None
    try:
        response = await driver.request(Request("GET", url))
        if isinstance(response.content, bytes):
            try:
                return response.content.decode("utf-8")
            except Exception as e:
                logger.opt(exception=e).warning(f"Failed to decode CSS from {url}")
        return None
    except Exception as e:
        logger.opt(exception=e).warning(f"Failed to fetch CSS resource from {url}")
    return None


async def combined_css_fetcher(url: str) -> str | None:
    content = await filesystem_css_fetcher(url)
    if content is not None:
        return content
    return await network_css_fetcher(url)


async def html_to_pic(
    html: str,
    *,
    base_url: str = "",
    dpi: float = 144.0,
    max_width: float = 800.0,
    device_height: float = 600.0,
    default_font_size: float = 12.0,
    font_name: str = "sans-serif",
    allow_refit: bool = True,
    lang: str = "zh",
    culture: str = "CN",
    img_fetch_fn: ImgFetchFn = combined_img_fetcher,
    css_fetch_fn: CSSFetchFn = combined_css_fetcher,
    urljoin_fn: Callable[[str, str], str] = urljoin,
) -> bytes:
    loop = get_running_loop()
    return await core._render_internal(  # pyright: ignore[reportPrivateUsage]
        html,
        base_url,
        dpi,
        max_width,
        device_height,
        default_font_size,
        font_name,
        allow_refit,
        lang,
        culture,
        lambda exc, exc_type, tb: nonebot.logger.opt(
            exception=(exc_type, exc, tb)
        ).error("Exception in html_to_pic: "),
        run_coroutine_threadsafe,
        urljoin_fn,
        loop,
        img_fetch_fn,
        css_fetch_fn,
    )


TEMPLATES_PATH = str(Path(__file__).parent / "templates")

env = jinja2.Environment(
    extensions=["jinja2.ext.loopcontrols"],
    loader=jinja2.FileSystemLoader(TEMPLATES_PATH),
    enable_async=True,
)


async def text_to_pic(
    text: str,
    css_path: str = "",
    *,
    max_width: int = 500,
    allow_refit: bool = True,
) -> bytes:
    """多行文本转图片

    Args:
        text (str): 纯文本, 可多行
        css_path (str, optional): css文件
        max_width (int, optional): 图片最大宽度，默认为 500
        allow_refit (bool, optional): 允许根据内容缩小宽度，默认为 True

    Returns:
        bytes: 图片, 可直接发送
    """
    template = env.get_template("text.html")

    return await html_to_pic(
        html=await template.render_async(
            text=text,
            css=await read_file(css_path) if css_path else await read_tpl("text.css"),
        ),
        max_width=max_width,
        base_url=f"file://{css_path or TEMPLATES_PATH}",
        allow_refit=allow_refit,
    )


async def md_to_pic(
    md: str = "",
    md_path: str = "",
    css_path: str = "",
    *,
    max_width: int = 500,
    img_fetch_fn: ImgFetchFn = combined_img_fetcher,
    allow_refit: bool = True,
) -> bytes:
    """markdown 转 图片

    Args:
        md (str, optional): markdown 格式文本
        md_path (str, optional): markdown 文件路径
        css_path (str,  optional): css文件路径. Defaults to None.
        max_width (int, optional): 图片最大宽度，默认为 500
        img_fetch_fn (FetchFn, optional): 图片获取函数，默认为 combined_fetcher
        allow_refit (bool, optional): 允许根据内容缩小宽度，默认为 True

    Returns:
        bytes: 图片, 可直接发送
    """
    template = env.get_template("markdown.html")
    if not md:
        if md_path:
            md = await read_file(md_path)
        else:
            raise Exception("md or md_path must be provided")
    logger.debug(md)
    md = markdown.markdown(
        md,
        extensions=[
            "pymdownx.tasklist",
            "tables",
            "fenced_code",
            "codehilite",
            "pymdownx.tilde",
        ],
        extension_configs={"mdx_math": {"enable_dollar_delimiter": True}},
    )

    logger.debug(md)
    if "math/tex" in md:
        logger.warning("TeX math is not supported by htmlkit.")

    if css_path:
        css = await read_file(css_path)
    else:
        css = await read_tpl("github-markdown-light.css") + await read_tpl(
            "pygments-default.css",
        )

    return await html_to_pic(
        html=await template.render_async(md=md, css=css),
        max_width=max_width,
        device_height=10,
        base_url=f"file://{css_path or TEMPLATES_PATH}",
        img_fetch_fn=img_fetch_fn,
        allow_refit=allow_refit,
    )


async def template_to_html(
    template_path: str,
    template_name: str,
    filters: None | dict[str, Any] = None,
    **kwargs,
) -> str:
    """使用jinja2模板引擎通过html生成图片

    Args:
        template_path (str): 模板路径
        template_name (str): 模板名
        filters (dict[str, Any] | None): 自定义过滤器
        **kwargs: 模板内容
    Returns:
        str: html
    """

    template_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(template_path),
        enable_async=True,
    )

    if filters:
        for filter_name, filter_func in filters.items():
            template_env.filters[filter_name] = filter_func
            logger.debug(f"Custom filter loaded: {filter_name}")

    template = template_env.get_template(template_name)

    return await template.render_async(**kwargs)


async def template_to_pic(
    template_path: str,
    template_name: str,
    templates: dict[Any, Any],
    filters: None | dict[str, Any] = None,
    *,
    max_width: int = 500,
    device_height: int = 10,
    base_url: str | None = None,
    img_fetch_fn: ImgFetchFn = combined_img_fetcher,
    css_fetch_fn: CSSFetchFn = combined_css_fetcher,
    allow_refit: bool = True,
) -> bytes:
    """使用jinja2模板引擎通过html生成图片

    Args:
        screenshot_timeout (float, optional): 截图超时时间，默认30000ms
        template_path (str): 模板路径
        template_name (str): 模板名
        templates (dict[Any, Any]): 模板内参数 如: {"name": "abc"}
        filters (dict[str, Any] | None): 自定义过滤器
        max_width (int, optional): 图片最大宽度，默认为 500
        device_height (int, optional): 设备高度，默认为 10，与实际图片高度无关
        base_url (str | None, optional): 基础路径，默认为 "file://{template_path}"
        img_fetch_fn (FetchFn, optional): 图片获取函数，默认为 combined_fetcher
        css_fetch_fn (FetchFn, optional): css获取函数，默认为 combined_fetcher
        allow_refit (bool, optional): 允许根据内容缩小宽度，默认为 True
    Returns:
        bytes: 图片 可直接发送
    """
    template_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(template_path),
        enable_async=True,
    )

    if filters:
        for filter_name, filter_func in filters.items():
            template_env.filters[filter_name] = filter_func
            logger.debug(f"Custom filter loaded: {filter_name}")

    template = template_env.get_template(template_name)

    return await html_to_pic(
        html=await template.render_async(**templates),
        base_url=base_url or f"file://{template_path}",
        max_width=max_width,
        device_height=device_height,
        img_fetch_fn=img_fetch_fn,
        css_fetch_fn=css_fetch_fn,
        allow_refit=allow_refit,
    )
