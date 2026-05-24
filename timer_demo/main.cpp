#include "Timer10ms.hpp"

#include <chrono>
#include <iostream>
#include <thread>

static void printTick(int& count, std::chrono::steady_clock::time_point ref, const char* tag)
{
    ++count;
    const auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - ref)
            .count();
    std::cout << tag << " [tick " << count << "] elapsed: " << elapsedMs << " ms\n";
}

int main()
{
    const auto globalStart = std::chrono::steady_clock::now();

    // ── Demo 1: StartTimeout (fires immediately, period = 10 ms) ──────────────
    std::cout << "=== Demo1: StartTimeout (immediate) ===\n";
    {
        int count = 0;
        Timer10ms timer;
        timer.SetCallback([&]() { printTick(count, globalStart, "[immediate]"); });
        timer.StartTimeout();
        std::this_thread::sleep_for(std::chrono::milliseconds(55));
        timer.StopTimeout();
        std::cout << "Demo1 total ticks: " << count << "\n\n";
    }

    // ── Demo 2: StartTimeoutAlignedTo (first tick at globalStart + 200 ms) ───
    // Mirrors StartTimeoutAlignWithBCN: caller specifies a reference time point;
    // the timer blocks until that moment, then fires every kPeriod.
    std::cout << "=== Demo2: StartTimeoutAlignedTo (aligned to t+200ms) ===\n";
    {
        int count = 0;
        Timer10ms timer;
        timer.SetCallback([&]() { printTick(count, globalStart, "[aligned] "); });

        const auto alignPoint = globalStart + std::chrono::milliseconds(200);
        timer.StartTimeoutAlignedTo(alignPoint);

        // Wait long enough to see a few aligned ticks.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        timer.StopTimeout();
        std::cout << "Demo2 total ticks: " << count << "\n";
    }

    return 0;
}
