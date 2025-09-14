from time import time_ns

from nonebot import get_driver, on_command, require
from nonebot.adapters.onebot.v11 import Message
from nonebot.adapters.onebot.v11.message import MessageSegment
from nonebot.drivers import HTTPClientMixin, Request
from nonebot.params import CommandArg

require("nonebot_plugin_htmlkit")
from nonebot_plugin_htmlkit import html_to_pic, md_to_pic, none_fetcher, text_to_pic

render = on_command("render", aliases={"渲染"}, priority=5)


@render.handle()
async def handle_render(content: Message = CommandArg()):
    html_content = content.extract_plain_text().strip()
    if not html_content:
        await render.finish("请提供要渲染的 HTML 内容。")

    try:
        image_bytes = await html_to_pic(html_content)
    except Exception as e:
        await render.finish(f"渲染失败: {e}")
    image_segment = MessageSegment.image(image_bytes)
    await render.finish(Message(image_segment))


render_md = on_command("render_md", aliases={"md", "markdown"}, priority=5)


@render_md.handle()
async def handle_render_md(content: Message = CommandArg()):
    md_content = content.extract_plain_text().strip()
    if not md_content:
        await render_md.finish("请提供要渲染的 Markdown 内容。")

    try:
        image_bytes = await md_to_pic(md_content)
    except Exception as e:
        await render_md.finish(f"渲染失败: {e}")
    image_segment = MessageSegment.image(image_bytes)
    await render_md.finish(Message(image_segment))


render_text = on_command("render_text", aliases={"text", "文本"}, priority=5)


@render_text.handle()
async def handle_render_text(content: Message = CommandArg()):
    text_content = content.extract_plain_text().strip()
    if not text_content:
        await render_text.finish("请提供要渲染的文本内容。")

    try:
        image_bytes = await text_to_pic(text_content)
    except Exception as e:
        await render_text.finish(f"渲染失败: {e}")
    image_segment = MessageSegment.image(image_bytes)
    await render_text.finish(Message(image_segment))


render_pep = on_command("render_pep", aliases={"pep"}, priority=5)


@render_pep.handle()
async def handle_render_pep(content: Message = CommandArg()):
    pep_number = content.extract_plain_text().strip()
    if not pep_number.isdigit():
        await render_pep.finish("请提供有效的 PEP 编号。")

    pep_url = f"https://peps.python.org/pep-{int(pep_number):04d}/"
    driver = get_driver()
    if not isinstance(driver, HTTPClientMixin):
        await render_pep.finish("HTTP 客户端不可用，无法获取 PEP 内容。")
    try:
        response = await driver.request(Request("GET", pep_url))

    except Exception as e:
        await render_pep.finish(f"渲染失败: {e}")
    if response.status_code != 200:
        await render_pep.finish(
            f"无法获取 PEP 内容，HTTP 状态码: {response.status_code}"
        )
    if isinstance(response.content, bytes):
        html_content = response.content.decode("utf-8")
    elif isinstance(response.content, str):
        html_content = response.content
    else:
        await render_pep.finish("无法解析 PEP 内容。")
    time_st = time_ns()
    image_bytes = await html_to_pic(
        html_content,
        base_url=pep_url,
        max_width=1000,
        css_fetch_fn=none_fetcher,
    )
    time_ed = time_ns()
    time_ms = (time_ed - time_st) / 1_000_000
    await render_pep.send(f"渲染耗时: {time_ms:.2f} ms")
    await render_pep.finish(MessageSegment.image(image_bytes))


render_man7 = on_command("render_man7", aliases={"man7"}, priority=5)


@render_man7.handle()
async def handle_render_man7(content: Message = CommandArg()):
    func_name = content.extract_plain_text().strip()
    _, manual = func_name.split(".", 1)
    manual_url = f"https://man7.org/linux/man-pages/man{manual}/{func_name}.html"
    driver = get_driver()
    if not isinstance(driver, HTTPClientMixin):
        await render_man7.finish("HTTP 客户端不可用，无法获取手册内容。")
    try:
        response = await driver.request(Request("GET", manual_url))
    except Exception as e:
        await render_man7.finish(f"渲染失败: {e}")
    if response.status_code != 200:
        await render_man7.finish(
            f"无法获取手册内容，HTTP 状态码: {response.status_code}"
        )
    if isinstance(response.content, bytes):
        html_content = response.content.decode("utf-8")
    elif isinstance(response.content, str):
        html_content = response.content
    else:
        await render_man7.finish("无法解析手册内容。")
    time_st = time_ns()
    image_bytes = await html_to_pic(
        html_content,
        base_url=manual_url,
        max_width=1000,
    )
    time_ed = time_ns()
    time_ms = (time_ed - time_st) / 1_000_000
    await render_man7.send(f"渲染耗时: {time_ms:.2f} ms")
    await render_man7.finish(MessageSegment.image(image_bytes))
