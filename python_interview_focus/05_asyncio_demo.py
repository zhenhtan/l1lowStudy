"""Python 面试重点 05: asyncio 并发基础"""

import asyncio


async def fetch_mock(name: str, delay: float) -> str:
    # await 让出控制权，体现协程并发
    await asyncio.sleep(delay)
    return f"{name} done"


async def async_demo() -> None:
    tasks = [
        fetch_mock("task_a", 0.2),
        fetch_mock("task_b", 0.1),
        fetch_mock("task_c", 0.3),
    ]
    results = await asyncio.gather(*tasks)
    print("async results:", results)


def run() -> None:
    asyncio.run(async_demo())
if __name__ == "__main__":
    run()
