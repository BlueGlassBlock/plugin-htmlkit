from nonebot import on_command, require
from nonebot.adapters.onebot.v11 import Message
from nonebot.adapters.onebot.v11.message import MessageSegment
from nonebot.params import CommandArg

require("nonebot_plugin_htmlkit")
from nonebot_plugin_htmlkit import html_to_pic

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
