#pragma once
#include <iostream>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadManager {
public:
    ThreadManager() : stop(false) {}

    ~ThreadManager() {
        stopAll();
    }

    // 启动线程，执行给定的函数
    void start(std::function<void()> func) {
        std::thread([this, func]() {
            func();
        }).detach();
    }

    // 启动线程，执行给定的类成员函数
    template <typename T>
    void start(T* instance, void (T::*memberFunc)()) {
        std::thread([this, instance, memberFunc]() {
            (instance->*memberFunc)();
        }).detach();
    }

    // 停止所有线程（仅用于示例，实际情况需要管理线程生命周期）
    void stopAll() {
        stop = true;
        cv.notify_all();
    }

    // 检查是否停止
    bool isStopped() const {
        return stop;
    }

private:
    std::atomic<bool> stop;
    std::condition_variable cv;
};