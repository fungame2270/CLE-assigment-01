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
    bool stop;

    void worker_thread();
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    template<typename F,typename... Args>
    auto enqueue(F&& f,Args&&... args) -> std::future<typename std::invoke_result<F,Args...>::type>;
};

#endif
