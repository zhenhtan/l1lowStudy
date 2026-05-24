"""Python 面试重点 07: 多线程与 GIL（threading vs multiprocessing）"""

import threading
import multiprocessing as mp
import time


def cpu_heavy(n: int) -> int:
    # CPU 密集任务: 用循环制造明显计算量
    s = 0
    for i in range(n):
        s += i * i
    return s


def io_like(delay: float) -> None:
    # IO 模拟任务: sleep 会释放 GIL
    time.sleep(delay)


def threading_cpu_demo(n: int) -> float:  # 结果通常接近单线程的两倍，因为 GIL 导致线程无法真正并行执行 CPU 密集任务
    result = [0, 0]

    def worker(idx: int) -> None:
        result[idx] = cpu_heavy(n)

    t0 = time.perf_counter()
    ts = [threading.Thread(target=worker, args=(0,)), threading.Thread(target=worker, args=(1,))]
    for t in ts:
        t.start()
    for t in ts:
        t.join()
    return time.perf_counter() - t0


def multiprocessing_cpu_demo(n: int) -> float:  # 多进程可以绕过 GIL，实现真正的并行
    t0 = time.perf_counter()
    with mp.Pool(processes=2) as pool:
        pool.map(cpu_heavy, [n, n]) # 第1个n是给第1个进程的，第2个n是给第2个进程的
    return time.perf_counter() - t0


def multiprocessing_pool_close_join_demo(n: int) -> list[int]:
    """不用 with 时：map 返回后显式 close + join，优雅结束所有 worker 进程。

    - close(): 不再往池里提交新任务，已提交的任务继续跑完。
    - join(): 阻塞直到所有 worker 进程退出（须在 close 之后调用）。
    """
    pool = mp.Pool(processes=2)
    try:
        results = pool.map(cpu_heavy, [n, n])
    finally:
        pool.close()
        pool.join()
    return results


def threading_io_demo() -> float:  # IO 密集任务，线程通常可以并行执行
    # 两个 0.5s 的 IO 任务，线程通常接近 0.5s 而不是 1.0s
    t0 = time.perf_counter()
    ts = [threading.Thread(target=io_like, args=(0.5,)), threading.Thread(target=io_like, args=(0.5,))]
    for t in ts:
        t.start()
    for t in ts:
        t.join()
    return time.perf_counter() - t0


def thread_process_demo() -> None:
    n = 2_000_000
    cpu_thread_cost = threading_cpu_demo(n)
    cpu_proc_cost = multiprocessing_cpu_demo(n)
    close_join_results = multiprocessing_pool_close_join_demo(n)
    io_thread_cost = threading_io_demo()

    print("CPU task by threading:", f"{cpu_thread_cost:.3f}s")
    print("CPU task by multiprocessing:", f"{cpu_proc_cost:.3f}s")
    print("IO-like task by threading:", f"{io_thread_cost:.3f}s")
    print("pool.close()+join() 示例结果:", close_join_results)
    print("结论: CPU 密集型优先 multiprocessing；IO 密集型 threading 通常足够。")


if __name__ == "__main__":
    thread_process_demo()
