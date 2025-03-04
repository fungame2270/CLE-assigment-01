#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>


class ThreadPool{
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable taskFinish;
    bool stop;

    void worker_thread();
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void join();

    template <typename F, typename... Args>
    void enqueue(F&& f, Args&&... args) {

        tasks.push([f = std::forward<F>(f), ...args = std::forward<Args>(args)]() {
            f(args...);
        });
        
        condition.notify_one();
    
    };
};

#endif
