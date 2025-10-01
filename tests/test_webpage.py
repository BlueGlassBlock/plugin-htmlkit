from pathlib import Path

import aiofiles
import pytest
from utils import assert_image_equal

PEP_7 = Path(__file__).parent / "PEP 7.html"


@pytest.mark.asyncio
@pytest.mark.parametrize("image_format", ["png", "jpeg"])
# @pytest.mark.parametrize("refit", [True, False])
# litehtml's refit algorithm crops code blocks too aggressively
async def test_render_pep(image_format, regen_ref, output_img_dir):
    from nonebot_plugin_htmlkit import html_to_pic

    async with aiofiles.open(PEP_7, encoding="utf-8") as f:
        html_content = await f.read()
    img_bytes = await html_to_pic(
        html_content,
        base_url=f"file://{PEP_7.absolute().as_posix()}",
        max_width=1440,
        image_format=image_format,
        allow_refit=False,
    )
    assert img_bytes.startswith(
        b"\x89PNG\r\n\x1a\n" if image_format == "png" else b"\xff\xd8"
    )

    filename = f"pep_7.{image_format}"
    await assert_image_equal(img_bytes, filename, regen_ref, output_img_dir)
