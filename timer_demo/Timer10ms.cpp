#include "Timer10ms.hpp"

#include <stdexcept>

void Timer10ms::StartTimeout()
{
    StartTimeoutAlignedTo(std::chrono::steady_clock::now() + kPeriod);
}

void Timer10ms::StartTimeoutAlignedTo(std::chrono::steady_clock::time_point alignPoint)
{
    if (running_.load())
    {
        return; // already running, no-op
    }
    running_.store(true);
    thread_ = std::thread(&Timer10ms::Run, this, alignPoint);
}

void Timer10ms::StopTimeout()
{
    running_.store(false);
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void Timer10ms::Run(std::chrono::steady_clock::time_point firstTick)
{
    auto next = firstTick;
    while (running_.load())
    {
        std::this_thread::sleep_until(next);
        next += kPeriod;

        if (running_.load() && callback_)
        {
            callback_();
        }
    }
}
