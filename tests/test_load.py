import pytest


@pytest.mark.asyncio
async def test_render_basic():
    from nonebot_plugin_htmlkit import html_to_pic

    img_bytes = await html_to_pic(
        "<html><body><h1>Hello, World!</h1><p>This is a test.</p></body></html>"
    )
    assert img_bytes.startswith(b"\x89PNG\r\n\x1a\n")
