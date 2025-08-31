import asyncio
from collections.abc import Coroutine
import concurrent.futures
from types import TracebackType
from typing import Any, Callable
from typing_extensions import TypeAlias, Unpack

def _init_fontconfig_internal() -> None: ...

_ExceptionTuple: TypeAlias = tuple[BaseException, type[BaseException], TracebackType]
_ExceptionHandleFn: TypeAlias = Callable[[Unpack[_ExceptionTuple]], None]
_AsyncioRunCoroutineThreadsafeFn: TypeAlias = Callable[
    [Coroutine[Any, Any, Any], asyncio.AbstractEventLoop],
    concurrent.futures.Future[Any],
]
_UrlJoinFn: TypeAlias = Callable[[str, str], str]
_FetchFn: TypeAlias = Callable[[str], Coroutine[Any, Any, None | bytes]]

def _render_internal(
    html_content: str,
    base_url: str,
    dpi: float,
    width: float,
    height: float,
    default_font_size: float,
    font_name: str,
    lang: str,
    culture: str,
    exception_fn: _ExceptionHandleFn,
    asyncio_run_coroutine_threadsafe: _AsyncioRunCoroutineThreadsafeFn,
    urljoin: _UrlJoinFn,
    loop: asyncio.AbstractEventLoop,
    img_fetch_fn: _FetchFn,
    css_fetch_fn: _FetchFn,
    /,
) -> asyncio.Future[bytes]: ...
