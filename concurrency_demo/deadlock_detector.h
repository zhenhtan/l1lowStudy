#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace deadlock {

class LockRegistry {
public:
    struct DeadlockInfo {
        std::vector<std::thread::id> cycle_threads;
        std::vector<std::string> waiting_mutexes;
    };

    static LockRegistry& instance();

    void register_mutex(int mutex_id, const std::string& name);

    void before_lock(int mutex_id);
    void cancel_wait(int mutex_id);
    void after_lock(int mutex_id);
    void after_unlock(int mutex_id);

    std::vector<DeadlockInfo> detect_deadlocks();
    std::string format_report(const std::vector<DeadlockInfo>& deadlocks) const;

private:
    LockRegistry() = default;

    static std::uint64_t tid_hash(std::thread::id tid);

    mutable std::mutex meta_mu_;
    std::unordered_map<int, std::string> mutex_names_;
    std::unordered_map<int, std::thread::id> mutex_owner_;
    std::unordered_map<std::thread::id, int> thread_waiting_mutex_;
    std::unordered_map<std::thread::id, std::vector<int>> thread_owned_mutexes_;
};

class TrackedMutex {
public:
    explicit TrackedMutex(std::string name);

    void lock();
    bool try_lock();
    void unlock();

    int id() const { return id_; }
    const std::string& name() const { return name_; }

private:
    static int next_id();

    int id_;
    std::string name_;
    std::mutex mu_;
};

class DeadlockMonitor {
public:
    using Callback = std::function<void(const std::string&)>;

    DeadlockMonitor(std::chrono::milliseconds interval, Callback callback);
    ~DeadlockMonitor();

    void start();
    void stop();

private:
    void run();

    std::chrono::milliseconds interval_;
    Callback callback_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

}  // namespace deadlock
