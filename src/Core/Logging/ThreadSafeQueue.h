#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace Liara::Logging
{

    template <typename T> class ThreadSafeQueue
    {
    private:
        mutable std::mutex m_mutex;
        std::queue<T> m_queue;
        std::condition_variable m_condition;

    public:
        ThreadSafeQueue() = default;

        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = default;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = default;

        void enqueue(T item) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(item));
            m_condition.notify_one();
        }

        bool dequeue(T& result) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) { return false; }

            result = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }

        std::optional<T> wait_and_dequeue(std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (m_condition.wait_for(lock, timeout, [this] { return !m_queue.empty(); })) {
                T result = std::move(m_queue.front());
                m_queue.pop();
                return result;
            }

            return std::nullopt;  // Timeout
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }
    };

}