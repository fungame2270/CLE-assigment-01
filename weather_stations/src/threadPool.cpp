#include "threadPool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();  // Wake up all workers

    for (std::thread &worker : threads) {
        if (worker.joinable()) 
            worker.join();
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) 
                return;

            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        taskFinish.notify_one();
    }
}

void ThreadPool::join() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        taskFinish.wait(lock, [this] { return tasks.empty(); });
        stop = true;
    }

    condition.notify_all();

    for (std::thread &worker : threads) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
