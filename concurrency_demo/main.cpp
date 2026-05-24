#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

int main()
{
    std::queue<int> tasks;
    std::mutex queueMutex;
    std::condition_variable cv;

    std::mutex printMutex;
    std::atomic<bool> done{false};
    std::atomic<int> processedCount{0};
    std::atomic<int> resultSum{0};

    constexpr int kTaskCount = 20;
    constexpr int kWorkerCount = 3;

    auto producer = std::thread([&]() {
        for (int i = 1; i <= kTaskCount; ++i)
        {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                tasks.push(i);
            }
            cv.notify_one();

            {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "[producer] push task " << i << '\n';
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        done.store(true);
        cv.notify_all();
    });

    std::vector<std::thread> workers;
    workers.reserve(kWorkerCount);

    for (int workerId = 0; workerId < kWorkerCount; ++workerId)
    {
        workers.emplace_back([&, workerId]() {
            while (true)
            {
                int task = 0;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    cv.wait(lock, [&]() { return done.load() || !tasks.empty(); });

                    if (tasks.empty())
                    {
                        if (done.load())
                        {
                            break;
                        }
                        continue;
                    }

                    task = tasks.front();
                    tasks.pop();
                }

                const int result = task * task;
                processedCount.fetch_add(1);
                resultSum.fetch_add(result);

                {
                    std::lock_guard<std::mutex> lock(printMutex);
                    std::cout << "[worker " << workerId << "] task=" << task
                              << " result=" << result << '\n';
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        });
    }

    producer.join();
    for (auto& t : workers)
    {
        t.join();
    }

    std::cout << "\n=== summary ===\n";
    std::cout << "processed count: " << processedCount.load() << '\n';
    std::cout << "result sum:      " << resultSum.load() << '\n';
    std::cout << "expected count:  " << kTaskCount << '\n';

    return 0;
}
