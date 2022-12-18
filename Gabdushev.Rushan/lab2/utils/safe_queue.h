#pragma once
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

// Many providers, single consumer
template <typename T>
class SafeQueue : private std::queue<T>
{
private:
    std::mutex access;
    std::condition_variable cv;

public:
    SafeQueue() = default;
    void waitForMessage(const std::chrono::microseconds &duration)
    {
        std::unique_lock lock(access);
        cv.wait_for(lock, duration);
    }
    std::size_t size()
    {
        std::scoped_lock lock(access);
        return this->std::queue<T>::size();
    }
    const T &front()
    {
        std::scoped_lock lock(access);
        return this->std::queue<T>::front();
    }
    void pop()
    {
        std::scoped_lock lock(access);
        this->std::queue<T>::pop();
    }
    void pop_repeat(const T &x)
    {
        std::scoped_lock lock(access);
        while (this->std::queue<T>::size() != 0 && this->std::queue<T>::front() == x)
        {
            this->std::queue<T>::pop();
        }
    }
    void push(const T &x)
    {
        {
            std::scoped_lock lock(access);
            this->std::queue<T>::push(x);
        }
        cv.notify_all();
    }
    void push(T &&x)
    {
        {
            std::scoped_lock lock(access);
            this->std::queue<T>::push(std::move(x));
        }
        cv.notify_all();
    }
};

template <typename T>
class SafeQueuePusher
{
private:
    std::shared_ptr<SafeQueue<T>> queue;
    SafeQueuePusher() = delete;

public:
    SafeQueuePusher(std::shared_ptr<SafeQueue<T>> &queue) : queue(queue){};
    void push(const T &x)
    {
        queue->push(x);
    }
};