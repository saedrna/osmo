#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T> class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    bool pop(T &item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        item = queue.front();
        queue.pop();
        return true;
    }
};