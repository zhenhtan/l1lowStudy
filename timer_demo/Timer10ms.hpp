#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

// Simplified periodic 10 ms timer demo.
// Mirrors the RtTimerBase pattern: StartTimeout / StopTimeout / callback on each tick.
class Timer10ms
{
public:
    static constexpr std::chrono::milliseconds kPeriod{10};

    Timer10ms() = default;
    ~Timer10ms() { StopTimeout(); }

    Timer10ms(const Timer10ms&) = delete;
    Timer10ms& operator=(const Timer10ms&) = delete;

    // Register the function to call on each 10 ms tick.
    void SetCallback(std::function<void()> cb) { callback_ = std::move(cb); }

    // Start periodic timer immediately.
    // Safe to call multiple times (no-op if already running).
    void StartTimeout();

    // Start periodic timer with the first tick aligned to alignPoint.
    // Mirrors StartTimeoutAlignWithBCN: waits until alignPoint, then fires every kPeriod.
    // If alignPoint is already in the past, the first tick fires immediately.
    void StartTimeoutAlignedTo(std::chrono::steady_clock::time_point alignPoint);

    // Stop the timer and wait for the worker thread to exit.
    void StopTimeout();

    bool IsRunning() const { return running_.load(); }

private:
    void Run(std::chrono::steady_clock::time_point firstTick);

    std::function<void()> callback_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};
