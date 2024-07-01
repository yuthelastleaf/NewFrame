#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class Semaphore {
public:
    Semaphore(int count = 0, int maxCount = 1) : count(count), maxCount(maxCount) {}

    void signal() {
        std::unique_lock<std::mutex> lock(mutex);
        if (count < maxCount) {
            ++count;
            cond_var.notify_one();
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, [this] { return count > 0; });
        --count;
    }

    bool try_wait() {
        std::unique_lock<std::mutex> lock(mutex);
        if (count > 0) {
            --count;
            return true;
        }
        return false;
    }

    bool timed_wait(int milliseconds) {
        std::unique_lock<std::mutex> lock(mutex);
        auto duration = std::chrono::milliseconds(milliseconds);
        if (cond_var.wait_for(lock, duration, [this] { return count > 0; })) {
            --count;
            return true;
        }
        return false;
    }

    void set_max_count(int new_max_count) {
        std::unique_lock<std::mutex> lock(mutex);
        maxCount = new_max_count;
        // 如果新的最大值大于当前信号量值，则可能需要通知等待的线程
        if (count < maxCount) {
            cond_var.notify_all();
        }
    }

private:
    std::mutex mutex;
    std::condition_variable cond_var;
    int count;
    int maxCount;
};
