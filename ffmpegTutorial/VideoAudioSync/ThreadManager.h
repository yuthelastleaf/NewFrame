#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>

class ThreadManager {
public:
    ThreadManager() : stopFlag(false), thread(nullptr) {}

    ~ThreadManager() {
        stop();
    }

    void start(std::function<void()> task) {
        if (thread == nullptr) {
            stopFlag = false;
            thread = new std::thread([this, task]() {
                while (!stopFlag) {
                    task();
                }
            });
        } else {
            std::cerr << "Thread already running!" << std::endl;
        }
    }

    void stop() {
        if (thread != nullptr) {
            stopFlag = true;
            if (thread->joinable()) {
                thread->join();
            }
            delete thread;
            thread = nullptr;
        }
    }

    bool isRunning() const {
        return thread != nullptr;
    }

private:
    std::atomic<bool> stopFlag;
    std::thread* thread;
};
