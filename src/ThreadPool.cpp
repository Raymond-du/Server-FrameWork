
#include "ThreadPool.h"

using namespace raymond;

ThreadPool::ThreadPool(int32_t num)
{       
        new (this)ThreadPool(); 
        createThreads(num);
}

ThreadPool::ThreadPool(/* args */)
{
        //获取核心数
        m_maxThreadsNums = std::thread::hardware_concurrency() * 2;
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::createThreads(const int& num){
        //创建线程要小于或等于最大的线程数
        for (int i = 0; i < num && m_threadsNum <= m_maxThreadsNums; i++)
        {
                //当前线程数和空闲线程++
		m_threadsNum++;
		m_idleThreadNum++; 
                
                //创建线程 使用emplace_back 由vector创建线程对象 
                m_workThreads.emplace_back([this]{
                        while (1)
                        {
                                std::function<void()> task;
                                {
                                        std::unique_lock<std::mutex> mutexLock(this->m_taskMutex);
                                        //当stop信号或存在task是继续执行,否则阻塞在这;
                                        this->m_taskCond.wait(mutexLock,[this]{
                                                return this->m_stopFlag || !this->m_tasks.empty();
                                        });
					if(this->m_stopFlag == true){
					    return;
					}
                                        if (this->m_joinFlag == true && this->m_tasks.empty()){
                                                return ;                        
                                        }
                                        task = std::move(this->m_tasks.front());
                                        this->m_tasks.pop();
                                        this->m_idleThreadNum --;
                                }
                                task(); 
                                //执行完任务,空闲线程加一
				this->m_idleThreadNum ++;
                        }
                });
        }
}

int ThreadPool::getThreadsNum(){
        return this->m_threadsNum;
}

void ThreadPool::join(){
        this->m_joinFlag = true;
        m_taskCond.notify_all();
        for(std::vector<std::thread>::iterator it = m_workThreads.begin(); it != m_workThreads.end();++it){
                (*it).join();
        }
}

void ThreadPool::stop(){
    this->m_stopFlag = true;
    m_taskCond.notify_all();
    for(std::vector<std::thread>::iterator it = m_workThreads.begin(); it != m_workThreads.end();++it){
	    (*it).join();
    }
}
