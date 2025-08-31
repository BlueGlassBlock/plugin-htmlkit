from asyncio import get_running_loop, run_coroutine_threadsafe
from collections.abc import Coroutine
from typing import Any, Callable, Optional
from urllib.parse import urljoin

import nonebot
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
def init(**kwargs: Any):
    nonebot.logger.info("Initializing fontconfig...")
    with config.set_fc_environ(get_plugin_config(FcConfig)):
        core._init_fontconfig_internal()  # pyright: ignore[reportPrivateUsage]
    nonebot.logger.info("Fontconfig initialized.")


async def _none_fetcher(_url: str) -> None:
    return None


FetchFn = Callable[[str], Coroutine[Any, Any, Optional[bytes]]]


async def html_to_pic(
    html_content: str,
    *,
    base_url: str = "",
    dpi: float = 96.0,
    max_width: float = 800.0,
    device_height: float = 600.0,
    default_font_size: float = 12.0,
    font_name: str = "sans-serif",
    lang: str = "zh",
    culture: str = "CN",
    img_fetch_fn: FetchFn = _none_fetcher,
    css_fetch_fn: FetchFn = _none_fetcher,
) -> bytes:
    loop = get_running_loop()
    return await core._render_internal(  # pyright: ignore[reportPrivateUsage]
        html_content,
        base_url,
        dpi,
        max_width,
        device_height,
        default_font_size,
        font_name,
        lang,
        culture,
        lambda exc, exc_type, tb: nonebot.logger.opt(
            exception=(exc_type, exc, tb)
        ).error("Exception in html_to_pic: "),
        run_coroutine_threadsafe,
        urljoin,
        loop,
        img_fetch_fn,
        css_fetch_fn,
    )
