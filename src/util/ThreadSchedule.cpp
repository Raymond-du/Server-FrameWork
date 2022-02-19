#include "ThreadSchedule.hpp"

#ifndef MAX_TASK
#define MAX_TASK 2048
#endif

namespace ray
{
TaskQueue::TaskQueue(size_t threshold)
    : m_taskSize(0)
    , m_unassocSize(0)
    , m_popPos(0)
    , m_pushPos(0)
{
    threshold = threshold > MAX_TASK ? MAX_TASK : threshold;
    m_tasks.resize(threshold, nullptr);
    m_threshold = threshold;
}

TaskQueue::~TaskQueue()
{
    for(auto it : m_tasks) {
        if(it != nullptr) {
            delete it;
        }
    }
    m_tasks.clear();
}

int TaskQueue::pushTask(const std::function<void()>& func, bool associated)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_taskSize == m_threshold) {
        return -1;
    }

    if(m_tasks[m_pushPos] == nullptr) {
        m_tasks[m_pushPos] = new TaskContect;
    }
    //±ÜÃâÁËpopÁËÏß³Ì·Ç¹ØÁªµÄtaskµ¼ÖÂ ²»Á¬Ðø
    while(m_tasks[m_pushPos]->m_isValid == true) {
        //µÈÓÚãÐÖµµÄÊ±ºòÇÐµ½µÚÒ»¸ö
        m_pushPos = m_pushPos == m_threshold ? 0 : m_pushPos + 1;
    }

    TaskContect* taskCtx = m_tasks[m_pushPos++];
    taskCtx->m_associated = associated;
    taskCtx->m_func = func;
    taskCtx->m_isValid = true;

    m_taskSize++;
    if(!associated) {
        m_unassocSize++;
    }
    return m_taskSize - 1;
}

optional<std::function<void()>> TaskQueue::popTask(bool associated)
{
    optional<std::function<void()>> ret;

    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_taskSize == 0 || (associated == false && m_unassocSize == 0)) {
        return ret;
    }
    auto pos = m_popPos;
    while(true) {
        if(m_tasks[pos]->m_isValid == true) {
            if(associated == false) {
                if(m_tasks[pos]->m_associated == false) {
                    ret = std::move(m_tasks[pos]->m_func);
                    m_tasks[pos]->m_isValid = false;
                    --m_unassocSize;
                    break;
                }
            } else {
                ret = std::move(m_tasks[pos]->m_func);
                m_tasks[pos]->m_isValid = false;
                m_popPos = pos;
                break;
            }
        }
        pos = pos == m_threshold ? 0 : pos + 1;
    }
    --m_taskSize;
    if(m_taskSize == 0) {
        m_popPos = 0;
        m_pushPos = 0;
    }
    return ret;
}

TaskQueueManager::TaskQueueManager(size_t size, size_t threshold)
    : m_queueCount(size)
    , m_taskQueues(size, new TaskQueue(threshold))
{
}

TaskQueueManager::~TaskQueueManager()
{
    for(auto it : m_taskQueues) {
        if(it != nullptr) {
            delete it;
        }
    }
}

int TaskQueueManager::pushTask(const std::function<void()>& func, bool associated, int id)
{
    int queueId = id % m_queueCount;
    return m_taskQueues[queueId]->pushTask(func, associated);
}

ray::optional<std::function<void()>> TaskQueueManager::popTask(int id)
{
    int queueId = id % m_queueCount;
    auto ret = m_taskQueues[queueId]->popTask(true);
    
    if (!ret) {        
        for(auto it : m_taskQueues) {
            ret = it->popTask(false);
            if(ret) {
                break;
            }
        }
    }
    return ret;
}

Thread::Thread(int workerID, TaskQueueManager* taskQueueMan)
    : m_workerID(workerID)
    , m_taskQueueMan(taskQueueMan)
    , m_stopFlag(false)
{
}

Thread::~Thread() { join(); }

void Thread::start()
{
    std::thread tmp(&Thread::workFunc_, this);
    m_thread.swap(tmp);
    m_stopFlag = false;
}

void Thread::join()
{
    m_stopFlag = true;
    if(m_thread.joinable()) {
        m_thread.join();
    }
}

void Thread::workFunc_()
{
    while(!m_stopFlag) {
        auto task = m_taskQueueMan->popTask(m_workerID);
        if(task) {
            task.getRef()();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

ThreadPool::ThreadPool(unsigned int tCount)
    : m_threadCount(tCount)
    , m_taskQueueMan(new TaskQueueManager(tCount, MAX_TASK))
{
}

ThreadPool::~ThreadPool() { join(); }

void ThreadPool::start()
{
    for (int i = 0; i < m_threadCount; ++i) {
        m_threads.push_back(new Thread(i, m_taskQueueMan));
    }
    for(auto& t : m_threads) {
        t->start();
    }
}

void ThreadPool::join()
{
    for(auto& t : m_threads) {
        t->join();
    }
}

int ThreadPool::addTask(const std::function<void()>& func, int taskId)
{
    bool isAssociated = taskId == -1 ? false : true;
    return m_taskQueueMan->pushTask(func, isAssociated, taskId);
}
}