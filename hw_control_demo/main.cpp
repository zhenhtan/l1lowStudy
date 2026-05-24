#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

struct SharedCounter
{
    // Adjacent atomics likely share cache line -> false sharing under contention.
    std::atomic<long long> value{0};
};

struct alignas(64) AlignedCounter
{
    // One counter per cache line to reduce cache-line ping-pong.
    std::atomic<long long> value{0};
};

template <typename Counter>
long long RunBenchmark(const int threadCount, const int iterations)
{
    std::vector<Counter> counters(static_cast<size_t>(threadCount));
    std::vector<std::thread> workers;
    workers.reserve(static_cast<size_t>(threadCount));

    const auto start = Clock::now();

    for (int tid = 0; tid < threadCount; ++tid)
    {
        workers.emplace_back([&, tid]() {
            for (int i = 0; i < iterations; ++i)
            {
                // Relaxed ordering: benchmark focuses on cache behavior, not synchronization semantics.
                counters[tid].value.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : workers)
    {
        t.join();
    }

    const auto end = Clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    long long sum = 0;
    for (const auto& c : counters)
    {
        sum += c.value.load(std::memory_order_relaxed);
    }

    std::cout << "sum=" << sum << ", elapsed=" << ms << " ms\n";
    return ms;
}

int main()
{
    const int threadCount = std::max(2u, std::thread::hardware_concurrency());
    const int iterations = 5'000'000;

    std::cout << "threads=" << threadCount << ", iterations/thread=" << iterations << "\n\n";

    std::cout << "[1] Without cache-line alignment (false sharing likely)\n";
    const auto t1 = RunBenchmark<SharedCounter>(threadCount, iterations);

    std::cout << "\n[2] With alignas(64) (reduced false sharing)\n";
    const auto t2 = RunBenchmark<AlignedCounter>(threadCount, iterations);

    std::cout << "\nSpeedup (aligned / shared): ";
    if (t2 > 0)
    {
        std::cout << static_cast<double>(t1) / static_cast<double>(t2) << "x\n";
    }
    else
    {
        std::cout << "N/A\n";
    }

    return 0;
}
