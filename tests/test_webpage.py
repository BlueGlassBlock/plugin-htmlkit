from pathlib import Path

import aiofiles
from img_comparer import load_image_bytes, mse
import pytest

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
    img = load_image_bytes(img_bytes)
    ref_path = Path(__file__).parent / "ref_images" / filename
    if regen_ref:
        async with aiofiles.open(ref_path, "wb") as f:
            await f.write(img_bytes)
        pytest.skip("Reference image regenerated, skipping verification")
    if output_img_dir:
        out_path = Path(output_img_dir)
        out_path.mkdir(exist_ok=True, parents=True)
        async with aiofiles.open(out_path / filename, "wb") as f:
            await f.write(img_bytes)
    assert ref_path.exists(), (
        f"Reference image {ref_path} does not exist. "
        "Run tests with --regen-ref to generate it."
    )
    async with aiofiles.open(ref_path, "rb") as f:
        ref_img_bytes = await f.read()
    ref_img = load_image_bytes(ref_img_bytes)
    assert (
        img.shape == ref_img.shape
    ), f"Image shape mismatch: got {img.shape}, expected {ref_img.shape}"
    error = mse(img, ref_img)
    assert error < 1.0, f"Image MSE too high: {error}"
