from contextlib import contextmanager
import os
from typing import Optional

from pydantic import BaseModel, Field

from nonebot.compat import model_fields


class FcConfig(BaseModel):
    """覆盖 Fontconfig 的配置选项

    参考 https://fontconfig.pages.freedesktop.org/fontconfig/fontconfig-user
    """

    fontconfig_file: Optional[str] = Field(
        default=None, description="覆盖默认的配置文件"
    )
    fontconfig_path: Optional[str] = Field(
        default=None, description="覆盖默认的配置目录"
    )
    fontconfig_sysroot: Optional[str] = Field(
        default=None, description="覆盖默认的 sysroot"
    )
    fc_debug: Optional[str] = Field(default=None, description="设置 debug 级别")
    fc_dbg_match_filter: Optional[str] = Field(
        default=None, description="当 FC_DEBUG 设置了 MATCH2 时，过滤 debug 输出"
    )
    fc_lang: Optional[str] = Field(
        default=None, description="设置默认语言，否则从 LOCALE 环境变量获取"
    )
    fontconfig_use_mmap: Optional[str] = Field(
        default=None, description="是否使用 mmap(2) 读取字体缓存"
    )


@contextmanager
def set_fc_environ(config: FcConfig):
    old_values = {}
    fields = model_fields(FcConfig)
    for field in fields:
        name = field.name.upper()
        value = getattr(config, field.name)
        if value is not None:
            old_values[name] = os.environ.get(name)
            os.environ[name] = value
    try:
        yield
    finally:
        for name, value in old_values.items():
            if value is None:
                del os.environ[name]
            else:
                os.environ[name] = value
