
#include "schedule.h"
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <mutex>
#include <sched.h>
#include <string>
#include "Logger.h"
#include "fiber.h"
#include "hook.h"
#include "macro.h"
#include "thread.h"
#include "util.h"

namespace raymond
{

	//协程调度器
	static thread_local raymond::FiberSchedule* t_schedule = nullptr;
	//当前线程的调度协程
	static thread_local raymond::Fiber::ptr t_schedule_fiber = nullptr;

	static Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");

	FiberSchedule::FiberSchedule(const std::string &name, bool useCurThread,size_t coreNum) 
		: m_name(name)
		, m_useCurThread(useCurThread)	{
		if (useCurThread)
		{
			//保证每个线程中智能有一个 调度器
			RAYMOND_ASSERT(t_schedule == nullptr);
			t_schedule = this;

			t_schedule_fiber = Fiber::getCurFiber();

			//    m_scheFiber.reset(new Fiber(std::bind(&FiberSchedule::scheduleFunc,this),false));
			Thread::setCurName(name + std::to_string(0));
		}
		//	//创建线程 // 调用start的时候在创建线程
		//	while(threadsNum-- > 0){
		//	   m_threads.emplace_back([]{
		//
		//	   });
		//	}
		RAYMOND_LOG_FMT_INFO(g_logger, "create FiberSchedule : %s", name.c_str());

		coreNum = coreNum > 0 ? coreNum : getCoreNum();
		start(coreNum);
	}

	FiberSchedule::~FiberSchedule()
	{
		if(m_joinFlag == false){			
			m_joinFlag = true;
			join();
		}
		RAYMOND_ASSERT(m_joinFlag);
		setScheduler(nullptr);
		setScheFiber(nullptr);
	}

	void FiberSchedule::setScheduler(FiberSchedule * sche){		
		t_schedule = sche;
	}

	FiberSchedule* FiberSchedule::getScheduler()
	{
		//确保存在调度器
		RAYMOND_ASSERT2(t_schedule, "Uninitialized scheduler");
		return t_schedule;

	}
	//存在问题
	Fiber::ptr FiberSchedule::getScheFiber()
	{
		RAYMOND_ASSERT2(t_schedule_fiber, "Uninitialized scheduler or Uninitialized Scheduler Fiber");
		return t_schedule_fiber;
	}

	void FiberSchedule::setScheFiber(Fiber::ptr fiber){
		t_schedule_fiber = fiber;
	}

	void FiberSchedule::schedule(Fiber::ptr fiber)
	{
		//如果调用了停止调度 则不能进行插入fiber
		//RAYMOND_ASSERT2(!m_stopFlag, "schedule is stopping");
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_fibers.push(fiber);
			RAYMOND_LOG_FMT_DEBUG(g_logger,"schedule %d fiber",fiber->getFiberId());
		}
		m_conVar.notify_one();
	}

	void FiberSchedule::start(uint32_t threadsNum)
	{
		//线程中 添加 thread_local的 调度对象
		for (size_t i = m_useCurThread ? 1 : 0; i < threadsNum; i++)
		{
			m_threads.emplace_back(new Thread(m_name + std::to_string(i), &FiberSchedule::scheduleFunc, this));
		}
	}
	//
	//hold 状态不知道如何处理
	void FiberSchedule::scheduleFunc()
	{
		RAYMOND_LOG_FMT_INFO(g_logger, "scheduler %s is running in thread %d", m_name.c_str(), Thread::getCurId());
		//设置线程局部变量		
		setScheduler(this);
		t_schedule_fiber = Fiber::getCurFiber();
		//允许线程hook
		set_hook_enable(true);
		
		//简单的执行过程  这里面fiber执行结束 需要将其删除
		Fiber::ptr fiber = nullptr;

		while (true)
		{
			fiber.reset();
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				//这里应该有一个条件变量 判断是否存在fiber
				m_conVar.wait(lock, [this] {
					//不为空的情况  和 join为true且没有活跃的线程
					return !this->m_fibers.empty() || (m_joinFlag && m_activeNum.load() <= 0);
				});
				
				if (RAYMOND_UNLIKELY(m_fibers.empty()))
				{
					//线程一个退出的时候,通知其他线程退出
					m_conVar.notify_all();
					break;
				}

				fiber = m_fibers.front();
				m_fibers.pop();
			}

			if (!fiber)
			{
				RAYMOND_LOG_FMT_WARN(g_logger, "Existence has been fiber that is null");
				continue;
			}

			if (fiber->getStatus() != Fiber::TERM &&
				fiber->getStatus() != Fiber::EXCEPT)
			{
				if(fiber->getStatus() == Fiber::HOLD){
					schedule(fiber);
					continue;
				}

				m_activeNum++;
				(*fiber).swapIn();

				//如果fiber 没有执行结束 再次加入到执行队列中&& m_joinFlag == false
				if (fiber->getStatus() == Fiber::READY )
				{
					schedule(fiber);
				}else{
					fiber.reset();
				}
				m_activeNum--;
				continue;
			}
		}
	}

	void FiberSchedule::join()
	{
		RAYMOND_LOG_FMT_INFO(g_logger, "Scheduler '%s' joined", m_name.c_str());
		m_joinFlag = true;
		//通知所有的线程进行停止
		// m_conVar.notify_all();

		for (auto &t : m_threads)
		{
			t->join();
		}
	}
	

} // namespace raymond
