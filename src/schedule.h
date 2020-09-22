
#ifndef __SCHEDULE_H
#define __SCHEDULE_H

#include "fiber.h"
#include <atomic>
#include <bits/stdint-uintn.h>
#include <complex>
#include <functional>
#include <memory>
#include <string>
#include <queue>
#include "thread.h"
#include <mutex>
#include <condition_variable>

namespace raymond{
    
    /**
     * @brief 协程的调度器
     */
    class FiberSchedule : public std::enable_shared_from_this<FiberSchedule>{
    private:
		std::condition_variable m_conVar;
		//线程池
		std::vector<Thread::ptr> m_threads;
		std::atomic<uint32_t> m_activeNum = {0};
		std::atomic<uint32_t> m_idleNum = {0};
		bool m_useCurThread = false;

		/**
		 * @brief start 启动协程调度
		 */
		void start(uint32_t threadsNum);
	protected:
		std::mutex m_mutex;
		//调度器的名字
		std::string m_name;
		//所有的协程
		std::queue<Fiber::ptr> m_fibers;
		//调度协程
		//Fiber::ptr m_scheFiber;
		//停止调度的标志
		bool m_joinFlag = false;
	
    public:
		typedef std::shared_ptr<FiberSchedule> ptr;
		/**
		 * @brief FiberSchedule 创建一个协程的调度器
		 *
		 * @param threadsNum 线程池的个数
		 * @param name	调度器的名字
		 * @param useCurThread	是否使用当前线程作为调度器线程,
		 * @param coreNum 启动核心数
		 *	如果为true 可能当前线程执行调度器的方法 默认为false;
		 */
		FiberSchedule(const std::string & name ,bool useCurThread = false,size_t coreNum = 0);

		virtual ~FiberSchedule();

		/**
		 * @brief scheduleFunc 调度函数
		 */
		void scheduleFunc();

		/**
		 * @brief getScheduler 获取全局的调度器
		 *
		 * @return 调度器的智能指针
		 */
		static FiberSchedule* getScheduler();

		/**
		 * @brief getScheFiber 获取调度协程
		 *
		 * @return 协程的智能指针
		 */
		static Fiber::ptr getScheFiber();

		/**
		 * @brief setScheFiber 设置调度器fiber
		 *
		 * @param fiber fiber对象
		 */
		static void setScheFiber(Fiber::ptr fiber);
		
		/**
		 * @brief setScheduler 设置线程调度器
		 *
		 * @param sche 调度器对象
		 */
		static void setScheduler(FiberSchedule * sche);

		/**
		 * @brief schedule 添加fiber到队列中
		 *
		 * @param fiber 需要添加的fiber
		 */
		void schedule(Fiber::ptr fiber);
		
		void schedule(std::function<void()> func){
			schedule(Fiber::ptr(new Fiber(func,1024*1024,true)));
		}

		/**
		 * @brief join 把现存的fiber执行完成,停止调度.
		 */
		void join();

		/**
		 * @brief getName 获取schedule的名称
		 *
		 * @return 字符串名称
		 */
		std::string getName() const {return m_name;}

	};
}

#endif
