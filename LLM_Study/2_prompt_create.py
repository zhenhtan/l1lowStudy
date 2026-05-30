# 导入依赖库
import dashscope
import os

# 从环境变量中获取 API Key

dashscope.api_key = "sk-fecdd35ed6fd405f83d0f3e20115a21a"

# 基于 prompt 生成文本
# 使用 deepseek-v3 模型
def get_completion(prompt, model="deepseek-v3"):
    messages = [{"role": "user", "content": prompt}]    # 将 prompt 作为用户输入
    response = dashscope.Generation.call(
        model=model,
        messages=messages,
        result_format='message',  # 将输出设置为message形式
        temperature=0,  # 模型输出的随机性，0 表示随机性最小
    )
    return response.output.choices[0].message.content  # 返回模型生成的文本
    
user_prompt = """
做一个手机流量套餐的客服代表，叫小瓜。可以帮助用户选择最合适的流量套餐产品。可以选择的套餐包括：
经济套餐，月费50元，10G流量；
畅游套餐，月费180元，100G流量；
无限套餐，月费300元，1000G流量；
校园套餐，月费150元，200G流量，仅限在校生。"""

instruction = """
你是一名专业的提示词创作者。你的目标是帮助我根据需求打造更好的提示词。

你将生成以下部分：
提示词：{根据我的需求提供更好的提示词}
优化建议：{用简练段落分析如何改进提示词，需给出严格批判性建议}
问题示例：{提出最多3个问题，以用于和用户更好的交流}
"""

prompt = f"""
# 目标
{instruction}

# 用户提示词
{user_prompt}
"""

response = get_completion(prompt)
print(response)