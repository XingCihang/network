#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <utility>
#include <optional>

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // 提交任务，返回 future
    //使用C++17，支持函数、Lambda表达式、std::function、成员函数指针、成员变量指针
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    // 等待所有任务执行完毕
    void wait_done();

    // 终止线程池
    void shutdown();

private:
    void worker_loop();

    std::vector<std::thread> workers;           //线程池的部分
    std::queue<std::function<void()>> tasks;    //任务队列

    std::mutex queue_mutex;                     //互斥锁
    std::condition_variable condition;          //条件变量

    std::atomic<bool> stop{false};              //原子的关闭标志位
    std::atomic<int> active_tasks{0};           //原子的活跃任务数量
};

// 构造函数
inline ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i)
        workers.emplace_back(&ThreadPool::worker_loop, this);
}

// 工作线程主循环
inline void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock lock(queue_mutex);
            //处理虚假唤醒
            condition.wait(lock, [this] {
                return stop || !tasks.empty();  //如果stop为空，或者任务队列为空，线程等待
            });

            if (stop && tasks.empty()) return;  //如果线程池要求stop，而且没有需要执行的任务，安全退出
            //取任务
            task = std::move(tasks.front());
            tasks.pop();
            ++active_tasks;
        }
        //执行
        task();
        --active_tasks;
    }
}

// 提交任务
template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    //返回值类型推导
    using ReturnType = std::invoke_result_t<F, Args...>;
    //任务封装
    auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    //获取结果
    std::future<ReturnType> result = task_ptr->get_future();

    {   //临界区中将任务加入队列：
        std::scoped_lock lock(queue_mutex); //C++17 的统一锁
        if (stop) {     //如果已经要关闭线程池了，就停止提交新任务了
            throw std::runtime_error("Cannot submit to stopped ThreadPool");
        }
        tasks.emplace([task_ptr]() {
            std::invoke(*task_ptr);     //std::invoke() 是 C++17 的万能函数调用器
        });
    }

    condition.notify_one(); //唤醒工作线程
    return result;
}

// 等待所有任务完成
inline void ThreadPool::wait_done() {
    while (true) {
        if (tasks.empty() && active_tasks.load() == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  //睡眠 1 毫秒
    }
}

// 优雅关闭线程池
inline void ThreadPool::shutdown() {
    {
        std::scoped_lock lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (auto& thread : workers)
        if (thread.joinable()) thread.join();
}

// 析构函数
inline ThreadPool::~ThreadPool() {
    shutdown();
}
