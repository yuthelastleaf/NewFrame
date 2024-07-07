#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>

template <typename T>
class Semaphore {
public:
    Semaphore(int maxCount = 10000) : maxCount(maxCount) {}

    // 添加资源到信号量
    void signal(T* resource) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (resources.size() < maxCount) {
            resources.push(resource);
            cond_var.notify_one();
        }
    }

    // 获取资源，阻塞直到有资源可用
    T* wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var.wait(lock, [this] { return !resources.empty(); });
        auto resource = resources.front();
        resources.pop();
        return resource;
    }

    // 尝试获取资源，不阻塞
    T* try_wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!resources.empty()) {
            auto resource = resources.front();
            resources.pop();
            return resource;
        }
        return nullptr;
    }

    // 尝试在指定时间内获取资源
    T* timed_wait(int milliseconds) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto duration = std::chrono::milliseconds(milliseconds);
        if (cond_var.wait_for(lock, duration, [this] { return !resources.empty(); })) {
            auto resource = resources.front();
            resources.pop();
            return resource;
        }
        return nullptr;
    }

    // 设置新的最大计数
    void set_max_count(int new_max_count) {
        std::unique_lock<std::mutex> lock(mutex_);
        maxCount = new_max_count;
        // 如果新的最大值大于当前信号量值，则可能需要通知等待的线程
        if (resources.size() < maxCount) {
            cond_var.notify_all();
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable cond_var;
    std::queue<T*> resources;
    int maxCount;
};
