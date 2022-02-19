
#include "fiber.h"
#include <atomic>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <memory>
#include <ucontext.h>
#include "Logger.h"
#include "macro.h"
#include "config.h"
#include "util.h"
#include "schedule.h"

namespace raymond
{
	//获取系统的logger
	static Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");
	static std::atomic<uint32_t> s_fiber_id{0};
	static std::atomic<uint32_t> s_fiber_count{0};
	// 当前执行的协程
	static thread_local Fiber::ptr t_curFiber = nullptr;
	//配置一下stacksize的默认大小
	static ConfigVar<uint32_t>::ptr s_fiber_stacksize = Config::lookUp<uint32_t>("fiber.stach_size", 1024 * 1024, "fiber stack size");

	class StackAllocator
	{
	public:
		/**
	 * @brief 申请size大小的堆空间作为栈
	 * 
	 * @param size  栈大小
	 * @return 栈空间的首地址
	 */
		static void *alloc(size_t size)
		{
			return malloc(size);
		}
		/**
	 * @brief 释放申请的栈空间
	 * 
	 */
		static void deAlloc(void *point, size_t size)
		{
			free(point);
		}
	};

	Fiber::Fiber() : m_id(++s_fiber_id)
	{
		m_status = EXEC;
		//当前线程执行的位置的上下文
		if (getcontext(&m_context))
		{
			RAYMOND_ASSERT2(false, "getcontext");
		}

		++s_fiber_count;
		RAYMOND_LOG_FMT_DEBUG(g_logger, "create main fiber");

		m_stack = nullptr;
		m_bySche = true;
	}

	Fiber::~Fiber()
	{
		--s_fiber_count;
		if (m_stack)
		{
			//如果协程的当前状态是TERM 和EXCEPT HOLD时 中断打印
			RAYMOND_ASSERT2(m_status != EXEC,std::to_string(m_id).c_str());
			StackAllocator::deAlloc(m_stack, m_stackSize);
		}
		else
		{
			//主协程的情况 没有栈
			RAYMOND_ASSERT(!m_func);
			RAYMOND_ASSERT(m_status == EXEC);

			if (t_curFiber.get() == this)
			{
				t_curFiber = nullptr;
			}
		}
		RAYMOND_LOG_FMT_DEBUG(g_logger, "filerid : %d end!", m_id);
	}

	Fiber::Fiber(std::function<void()> func, size_t stacksize, bool bySche)
		: m_id(++s_fiber_id), 
		m_func(func), 
		m_bySche(bySche){

		m_stackSize = stacksize ? stacksize : s_fiber_stacksize->getValue();
		m_stack = StackAllocator::alloc(m_stackSize);
		if (getcontext(&m_context))
		{
			RAYMOND_LOG_FMT_ERROR(g_logger, "getCurFiber error");
		}

		m_context.uc_stack.ss_sp = m_stack;
		m_context.uc_stack.ss_size = m_stackSize;

		RAYMOND_LOG_FMT_DEBUG(g_logger, "create fiber %d !", m_id);
	}

	void Fiber::reset(std::function<void()> func)
	{
		//先判断栈是否存在
		RAYMOND_ASSERT(m_stack);
		//判断当前协程执行的状态
		RAYMOND_ASSERT(m_status == INIT || m_status == EXCEPT || m_status == TERM);

		m_func = func;
		//重置 该协程的ucontext
		if (getcontext(&m_context))
		{
			RAYMOND_ASSERT2(false, "getcontext");
		}

		m_context.uc_link = &(m_mainFiber->m_context);
		m_context.uc_stack.ss_sp = m_stack;
		m_context.uc_stack.ss_size = m_stackSize;

		makecontext(&m_context, &Fiber::mainFunc, 0);

		RAYMOND_LOG_FMT_DEBUG(g_logger, "fiber id %d has reset", m_id);
	}

	void Fiber::swapIn()
	{
		//当前的协程不能在运行中
		if (m_status == EXEC)
		{
			return;
		}

		//不服从 调度器调度时
		if (!m_mainFiber)
		{
			if (!m_bySche)
			{
				m_mainFiber = getCurFiber();
			}
			else
			{
				m_mainFiber = FiberSchedule::getScheFiber();
			}
			m_context.uc_link = &(m_mainFiber->m_context);
			makecontext(&m_context, &Fiber::mainFunc, 0);
		}

		t_curFiber = this->shared_from_this();
		m_status = EXEC;

		RAYMOND_LOG_FMT_DEBUG(g_logger, "fiber id %d swapin", m_id);
		if (swapcontext(&(m_mainFiber->m_context), &m_context))
		{
			RAYMOND_ASSERT2(false, "swapcontext");
		}
	}

	void Fiber::swapOut()
	{
		t_curFiber = m_mainFiber;
		if(m_status != HOLD){
			m_status = READY;
		}

		RAYMOND_LOG_FMT_DEBUG(g_logger, "fiber id %d swapOut", m_id);
		if (swapcontext(&m_context, &(m_mainFiber->m_context)))
		{
			RAYMOND_ASSERT2(false, "swapcontext");
		}
	}

	void Fiber::swapToHold(){
		t_curFiber = m_mainFiber;

		m_status = HOLD;
		RAYMOND_LOG_FMT_DEBUG(g_logger,"fiber id %d swapToHold",m_id);

		if(swapcontext(&m_context,&(m_mainFiber->m_context))){
			RAYMOND_LOG_FMT_ERROR(g_logger,"error : %s",std::strerror(errno));
			RAYMOND_ASSERT2(false,"swapcontext");
		}
	}

	Fiber::ptr Fiber::getCurFiber()
	{
		//如果存在当前协程,则直接返回,如果不存在则创建主协程
		if (t_curFiber)
		{
			return t_curFiber->shared_from_this();
		}
		Fiber::ptr curFiber(new Fiber());
		curFiber->m_mainFiber = FiberSchedule::getScheFiber();
		t_curFiber = curFiber;
		return curFiber;
	}

	void Fiber::yeildToReady()
	{
		Fiber::ptr curFiber = getCurFiber();

		RAYMOND_ASSERT(curFiber->m_status == EXEC);
		curFiber->swapOut();
	}

	void Fiber::yeildToHold()
	{
		Fiber::ptr curFiber = getCurFiber();
		RAYMOND_ASSERT(curFiber->m_status == EXEC);
		curFiber->swapToHold();
	}

	uint32_t Fiber::getFibersNum()
	{
		return s_fiber_count;
	}

	//设置当前的协程
	// void Fiber::setCurFiber(Fiber::ptr fiber){
	// t_curFiber = fiber;
	// }

	void Fiber::mainFunc()
	{
		Fiber *curFiber = getCurFiber().get();
		RAYMOND_ASSERT(curFiber);

		try
		{
			curFiber->m_status = EXEC;
			curFiber->m_func();
			curFiber->m_status = TERM;
		}
		catch (std::exception &e)
		{
			curFiber->m_status = EXCEPT;
			RAYMOND_LOG_FMT_ERROR(g_logger, "fiber 执行出错");
			RAYMOND_LOG_FMT_ERROR(g_logger, "Fiber except : fiber id %d,\n%s error:%s",e.what(), curFiber->m_id, backTraceToString().c_str());
		}
		RAYMOND_LOG_FMT_INFO(g_logger, "fiber id %d has finished", curFiber->m_id);
	}

	Fiber::Status Fiber::getStatus()
	{
		return m_status;
	}

	uint32_t Fiber::getFiberId(){
		return m_id;
	}

	void Fiber::setStatus(Fiber::Status status)
	{
		m_status = status;
	}

	std::function<void ()> Fiber::getFunc(){
		return m_func;
	}
} // namespace raymond
