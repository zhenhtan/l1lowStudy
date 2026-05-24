#include "deadlock_detector.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace deadlock {

LockRegistry& LockRegistry::instance()
{
    static LockRegistry registry;
    return registry;
}

void LockRegistry::register_mutex(int mutex_id, const std::string& name)
{
    std::lock_guard<std::mutex> lock(meta_mu_);
    mutex_names_[mutex_id] = name;
}

void LockRegistry::before_lock(int mutex_id)
{
    std::lock_guard<std::mutex> lock(meta_mu_);
    thread_waiting_mutex_[std::this_thread::get_id()] = mutex_id;
}

void LockRegistry::cancel_wait(int mutex_id)
{
    const auto tid = std::this_thread::get_id();
    std::lock_guard<std::mutex> lock(meta_mu_);

    auto it = thread_waiting_mutex_.find(tid);
    if (it != thread_waiting_mutex_.end() && it->second == mutex_id) {
        thread_waiting_mutex_.erase(it);
    }
}

void LockRegistry::after_lock(int mutex_id)
{
    const auto tid = std::this_thread::get_id();
    std::lock_guard<std::mutex> lock(meta_mu_);

    thread_waiting_mutex_.erase(tid);
    mutex_owner_[mutex_id] = tid;
    thread_owned_mutexes_[tid].push_back(mutex_id);
}

void LockRegistry::after_unlock(int mutex_id)
{
    const auto tid = std::this_thread::get_id();
    std::lock_guard<std::mutex> lock(meta_mu_);

    auto owner_it = mutex_owner_.find(mutex_id);
    if (owner_it != mutex_owner_.end() && owner_it->second == tid) {
        mutex_owner_.erase(owner_it);
    }

    auto owned_it = thread_owned_mutexes_.find(tid);
    if (owned_it != thread_owned_mutexes_.end()) {
        auto& vec = owned_it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), mutex_id), vec.end());
        if (vec.empty()) {
            thread_owned_mutexes_.erase(owned_it);
        }
    }
}

std::vector<LockRegistry::DeadlockInfo> LockRegistry::detect_deadlocks()
{
    std::unordered_map<std::thread::id, int> waiting_snapshot;
    std::unordered_map<int, std::thread::id> owner_snapshot;
    std::unordered_map<int, std::string> names_snapshot;

    {
        std::lock_guard<std::mutex> lock(meta_mu_);
        waiting_snapshot = thread_waiting_mutex_;
        owner_snapshot = mutex_owner_;
        names_snapshot = mutex_names_;
    }

    std::unordered_map<std::thread::id, std::thread::id> wait_for_edge;
    wait_for_edge.reserve(waiting_snapshot.size());

    for (const auto& [tid, waiting_mutex] : waiting_snapshot) {
        auto owner_it = owner_snapshot.find(waiting_mutex);
        if (owner_it == owner_snapshot.end()) {
            continue;
        }
        const auto owner_tid = owner_it->second;
        if (owner_tid == tid) {
            continue;
        }
        wait_for_edge[tid] = owner_tid;
    }

    std::unordered_map<std::thread::id, int> state;
    std::unordered_set<std::string> seen_cycles;
    std::vector<DeadlockInfo> deadlocks;

    for (const auto& [start_tid, _] : wait_for_edge) {
        if (state[start_tid] != 0) {
            continue;
        }

        std::vector<std::thread::id> path;
        std::unordered_map<std::thread::id, std::size_t> index_in_path;

        std::thread::id cur = start_tid;
        while (true) {
            if (state[cur] == 2) {
                break;
            }

            auto idx_it = index_in_path.find(cur);
            if (idx_it != index_in_path.end()) {
                std::vector<std::thread::id> cycle(path.begin() + static_cast<long>(idx_it->second), path.end());
                if (cycle.size() >= 2) {
                    std::vector<std::uint64_t> hashes;
                    hashes.reserve(cycle.size());
                    for (const auto& tid : cycle) {
                        hashes.push_back(tid_hash(tid));
                    }
                    std::sort(hashes.begin(), hashes.end());
                    std::ostringstream key;
                    for (const auto h : hashes) {
                        key << h << ",";
                    }

                    if (seen_cycles.insert(key.str()).second) {
                        DeadlockInfo info;
                        info.cycle_threads = cycle;
                        for (const auto& tid : cycle) {
                            auto w_it = waiting_snapshot.find(tid);
                            if (w_it == waiting_snapshot.end()) {
                                info.waiting_mutexes.emplace_back("<unknown>");
                                continue;
                            }
                            const int mid = w_it->second;
                            auto n_it = names_snapshot.find(mid);
                            if (n_it != names_snapshot.end()) {
                                info.waiting_mutexes.push_back(n_it->second);
                            } else {
                                info.waiting_mutexes.push_back("mutex#" + std::to_string(mid));
                            }
                        }
                        deadlocks.push_back(std::move(info));
                    }
                }
                break;
            }

            auto edge_it = wait_for_edge.find(cur);
            if (edge_it == wait_for_edge.end()) {
                break;
            }

            index_in_path[cur] = path.size();
            path.push_back(cur);
            cur = edge_it->second;
        }

        for (const auto& tid : path) {
            state[tid] = 2;
        }
    }

    return deadlocks;
}

std::string LockRegistry::format_report(const std::vector<DeadlockInfo>& deadlocks) const
{
    if (deadlocks.empty()) {
        return "";
    }

    std::ostringstream oss;
    oss << "\n[DeadlockDetector] detected " << deadlocks.size() << " deadlock cycle(s)\n";
    for (std::size_t i = 0; i < deadlocks.size(); ++i) {
        const auto& d = deadlocks[i];
        oss << "  cycle " << (i + 1) << ":\n";
        for (std::size_t j = 0; j < d.cycle_threads.size(); ++j) {
            const auto tid = d.cycle_threads[j];
            const auto next_tid = d.cycle_threads[(j + 1) % d.cycle_threads.size()];
            const std::string& waiting_on = d.waiting_mutexes[j];
            oss << "    T" << tid_hash(tid) << " waits " << waiting_on
                << " held by T" << tid_hash(next_tid) << "\n";
        }
    }
    return oss.str();
}

std::uint64_t LockRegistry::tid_hash(std::thread::id tid)
{
    return static_cast<std::uint64_t>(std::hash<std::thread::id>{}(tid));
}

TrackedMutex::TrackedMutex(std::string name)
    : id_(next_id()), name_(std::move(name))
{
    LockRegistry::instance().register_mutex(id_, name_);
}

void TrackedMutex::lock()
{
    LockRegistry::instance().before_lock(id_);
    mu_.lock();
    LockRegistry::instance().after_lock(id_);
}

bool TrackedMutex::try_lock()
{
    LockRegistry::instance().before_lock(id_);
    if (!mu_.try_lock()) {
        LockRegistry::instance().cancel_wait(id_);
        return false;
    }
    LockRegistry::instance().after_lock(id_);
    return true;
}

void TrackedMutex::unlock()
{
    LockRegistry::instance().after_unlock(id_);
    mu_.unlock();
}

int TrackedMutex::next_id()
{
    static std::atomic<int> id_gen{1};
    return id_gen.fetch_add(1);
}

DeadlockMonitor::DeadlockMonitor(std::chrono::milliseconds interval, Callback callback)
    : interval_(interval), callback_(std::move(callback))
{
}

DeadlockMonitor::~DeadlockMonitor()
{
    stop();
}

void DeadlockMonitor::start()
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        return;
    }
    worker_ = std::thread(&DeadlockMonitor::run, this);
}

void DeadlockMonitor::stop()
{
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false)) {
        return;
    }
    if (worker_.joinable()) {
        worker_.join();
    }
}

void DeadlockMonitor::run()
{
    while (running_.load()) {
        std::this_thread::sleep_for(interval_);
        const auto deadlocks = LockRegistry::instance().detect_deadlocks();
        if (!deadlocks.empty() && callback_) {
            callback_(LockRegistry::instance().format_report(deadlocks));
        }
    }
}

}  // namespace deadlock
