#include <iostream>
#include "ThrdPool.hpp"

void sample_task(int id) {
    std::cout << "Task " << id << " running on thread " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    ThreadPool pool(4);  // 创建 4 个线程的线程池

    for (int i = 0; i < 10; ++i) {
        pool.submit(sample_task, i);
    }

    pool.wait_done();    // 等待所有任务完成
    return 0;
}
