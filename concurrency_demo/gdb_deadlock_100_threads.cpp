#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <pthread.h>
#include <unistd.h>

using namespace std::chrono_literals;

namespace {

std::mutex g_mutex_a;
std::mutex g_mutex_b;
std::atomic<bool> g_stop{false};

void set_thread_name(const char* name)
{
    // Linux 线程名最大 15 字符（不含 '\0'）。
    pthread_setname_np(pthread_self(), name);
}

void on_signal(int)
{
    g_stop.store(true);
}

void normal_worker(int idx)
{
    char name[16] = {0};
    std::snprintf(name, sizeof(name), "wk_%02d", idx);
    set_thread_name(name);

    while (!g_stop.load()) {
        std::this_thread::sleep_for(200ms);
    }
}

void deadlock_thread_1()
{
    set_thread_name("deadlock_t1");

    std::unique_lock<std::mutex> lock_a(g_mutex_a);
    std::this_thread::sleep_for(300ms);

    // 等待 g_mutex_b（被 deadlock_t2 持有）
    std::unique_lock<std::mutex> lock_b(g_mutex_b);
    (void)lock_b;
}

void deadlock_thread_2()
{
    set_thread_name("deadlock_t2");

    std::unique_lock<std::mutex> lock_b(g_mutex_b);
    std::this_thread::sleep_for(300ms);

    // 等待 g_mutex_a（被 deadlock_t1 持有）
    std::unique_lock<std::mutex> lock_a(g_mutex_a);
    (void)lock_a;
}

}  // namespace

int main()
{
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    std::cout << "PID=" << getpid() << "\n";
    std::cout << "Start 100 threads: 98 normal + 2 deadlock threads.\n";
    std::cout << "Use gdb to inspect deadlock threads.\n";

    std::vector<std::thread> threads;
    threads.reserve(100);

    for (int i = 0; i < 98; ++i) {
        threads.emplace_back(normal_worker, i);
    }

    threads.emplace_back(deadlock_thread_1);
    threads.emplace_back(deadlock_thread_2);

    // 主线程保持存活，方便 gdb attach。
    while (!g_stop.load()) {
        std::this_thread::sleep_for(500ms);
    }

    // 注意：两个死锁线程无法正常退出，这里仅用于演示，可直接 kill -9 结束。
    for (auto& t : threads) {
        if (t.joinable()) {
            t.detach();
        }
    }

    std::cout << "Exit requested.\n";
    return 0;
}
