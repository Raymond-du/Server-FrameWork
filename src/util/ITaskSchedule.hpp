#ifndef __UTIL_ITASKSCHEDULE_HPP
#define __UTIL_ITASKSCHEDULE_HPP

#include <functional>

namespace ray
{
class IWorker
{
public:
    virtual ~IWorker() = default;
    virtual void start() = 0;
    virtual void join() = 0;
};

class IWorkerPool
{
public:
    virtual ~IWorkerPool() = default;
    virtual void start() = 0;
    virtual void join() = 0;
    virtual int addTask(const std::function<void()>& func, int taskId = -1) = 0;
};
}

#endif //__UTIL_ITASKSCHEDULE_HPP