import nonebot
from nonebot.adapters.onebot.v11 import Adapter as OB11Adapter

# 初始化 NoneBot
nonebot.init()

# 注册适配器
driver = nonebot.get_driver()
driver.register_adapter(OB11Adapter)

# 在这里加载插件
nonebot.load_builtin_plugins("echo")
nonebot.load_plugin("nonebot_plugin_htmlkit")
nonebot.load_plugin("render_demo")

if __name__ == "__main__":
    nonebot.run()
