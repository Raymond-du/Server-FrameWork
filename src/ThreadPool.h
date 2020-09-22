
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <sys/types.h>
#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <mutex>
#include<condition_variable>
#include<memory>
#include <atomic>

#include <iostream>

namespace raymond
{
        //线程池
        class ThreadPool
        {
        private:
                //设置可以开最大的线程数 核心数的2倍
                unsigned int m_maxThreadsNums = 0;
                //当前线程数
		std::atomic<uint32_t> m_threadsNum = {0};
                //线程空闲的个数;
		std::atomic<uint32_t> m_idleThreadNum = {0};
                //工作线程队列;
                std::vector<std::thread> m_workThreads;
                //任务队列    使用保存std::packaged_task 重载了()函数
                std::queue<std::function<void()>> m_tasks;
                //任务互斥所
                std::mutex m_taskMutex;
                //条件锁 判断是否有任务;
                std::condition_variable m_taskCond;
                //停止标志
                bool m_stopFlag = false;
		//join标志
		bool m_joinFlag = false;
        public:
                //默认线程个数等于CPU核心数
                ThreadPool(/* args */);
                //线程个数
                ThreadPool(int32_t num);
                //获取线程池中的个数
                int getThreadsNum();
                //添加任务 返回一个future
                template<class F,class... Args>
                auto excute(F &&f, Args && ... args) 
                        -> std::future<typename std::result_of<F(Args...)>::type>;
                //执行完所有的任务再跳出
                void join();
		//执行玩运行的程序就退出
		void stop();

                ~ThreadPool();

        private:
                void createThreads(const int& num);
        };

        template<class F,class... Args>
        auto ThreadPool::excute(F&& f,Args&&... args)
                ->std::future<typename std::result_of<F(Args...)>::type>{
                //F函数的返回值类型
                using return_type = typename std::result_of<F(Args...)>::type;

                //将执行的函数 封装到task中
                auto task = std::make_shared<std::packaged_task<return_type()>>(
                        std::bind(std::forward<F>(f),std::forward<Args>(args)...));

                std::future<return_type> res = task->get_future();
                //判断空闲线程还有多少
                if(m_idleThreadNum < 2)
                        createThreads(2);
                {
                        std::unique_lock<std::mutex> lock(m_taskMutex);
                        //添加到 任务队列中
                        m_tasks.emplace([task]{
                                (*task)();
                        });
                }
                
                m_taskCond.notify_one();
                return res;
        }


} // namespace raymond

#endif // THREADPOOL_H
