#ifndef __UTIL_THREADSCHEDULE_HPP
#define __UTIL_THREADSCHEDULE_HPP

#include "ITaskSchedule.hpp"
#include "NoCopyable.hpp"
#include "Optional.hpp"
#include <mutex>
#include <thread>
#include <vector>

namespace ray
{

class TaskQueue
{
public:
    TaskQueue(size_t threshold);
    ~TaskQueue();
    int pushTask(const std::function<void()>& func, bool associated);
    ray::optional<std::function<void()>> popTask(bool associated);

private:
    struct TaskContect {
        bool m_isValid = false;
        bool m_associated; //线程关联的
        std::function<void()> m_func;
    };

    unsigned short m_threshold;
    unsigned short m_taskSize;
    unsigned short m_unassocSize; //不关联的task个数
    unsigned short m_popPos;
    unsigned short m_pushPos;
    std::vector<TaskContect*> m_tasks;
    std::mutex m_mutex;
};

class TaskQueueManager
{
public:
    TaskQueueManager(size_t size, size_t threshold);
    ~TaskQueueManager();
    int pushTask(const std::function<void()>& func, bool associated, int id);
    ray::optional<std::function<void()>> popTask(int id);

private:
    size_t m_queueCount;
    std::vector<TaskQueue*> m_taskQueues;
};

class Thread : public IWorker, NoCopyable
{
public:
    Thread(int workerID, TaskQueueManager* taskQueueMan);
    ~Thread();
    void start() override;
    void join() override;

private:
    void workFunc_();

private:
    bool m_stopFlag;
    int m_workerID;
    TaskQueueManager* m_taskQueueMan;
    std::thread m_thread;
};

class ThreadPool : public IWorkerPool, NoCopyable
{
public:
    ThreadPool(unsigned int tCount);
    ~ThreadPool();
    void start() override;
    void join() override;
    int addTask(const std::function<void()>& func, int taskId = -1) override;

private:
    int m_threadCount;
    std::vector<Thread*> m_threads;
    TaskQueueManager* m_taskQueueMan;
};

}

#endif //__UTIL_THREADSCHEDULE_HPP