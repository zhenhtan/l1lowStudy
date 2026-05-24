#include "deadlock_detector.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

int main()
{
    deadlock::TrackedMutex m1("account_mutex");
    deadlock::TrackedMutex m2("orderbook_mutex");

    deadlock::DeadlockMonitor monitor(200ms, [](const std::string& msg) {
        std::cout << msg;
    });
    monitor.start();

    std::vector<std::thread> normal_workers;
    normal_workers.reserve(98);

    for (int i = 0; i < 98; ++i) {
        normal_workers.emplace_back([i]() {
            deadlock::TrackedMutex local("worker_mutex_" + std::to_string(i));
            for (int k = 0; k < 100; ++k) {
                std::lock_guard<deadlock::TrackedMutex> lk(local);
                std::this_thread::sleep_for(1ms);
            }
        });
    }

    std::thread t1([&]() {
        std::lock_guard<deadlock::TrackedMutex> lock1(m1);
        std::this_thread::sleep_for(100ms);
        std::lock_guard<deadlock::TrackedMutex> lock2(m2);
        (void)lock2;
    });

    std::thread t2([&]() {
        std::lock_guard<deadlock::TrackedMutex> lock1(m2);
        std::this_thread::sleep_for(100ms);
        std::lock_guard<deadlock::TrackedMutex> lock2(m1);
        (void)lock2;
    });

    for (auto& w : normal_workers) {
        w.join();
    }

    // 两个线程会一直卡住，这里不 join，留给监控线程输出死锁对
    t1.detach();
    t2.detach();

    std::this_thread::sleep_for(2000s);
    monitor.stop();

    std::cout << "\nDemo done. If deadlock exists, it has been reported above.\n";
    return 0;
}
