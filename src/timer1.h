
#ifndef __TIMER_H
#define __TIMER_H

#include "RWMutex.h"
#include "fiber.h"
#include <bits/stdint-uintn.h>
#include <bits/types/struct_timespec.h>
#include <functional>
#include <memory>
#include <algorithm>
#include <thread>
#include <vector>
#include <signal.h>
#include <sys/time.h>
#include <sys/time.h>

namespace raymond{
	class TimerManager;

	/**
	 * @brief 使用linux alarm进行创建一个定时器 将 定时的任务交给schedule
	 */
	class Timer : public std::enable_shared_from_this<Timer>{
	friend class TimerManager;

	private:
		//std::function<void()> m_func;
		std::function<void()> m_func;
		timespec m_triggerTime;		//第一次触发定时器的时间间隔
		timespec m_intervalTime;	//周期触发定时器的间隔
		timespec m_execTime;	//执行的准确时间
	public:
		typedef std::shared_ptr<Timer> ptr;

		bool isRecurring();
		
		/**
		 * @brief Timer 定时器的构造函数
		 *
		 * @param ms	第一次触发的时间
		 * @param func	触发后执行的方法
		 * @param interval	循环触发的间隔 (如果为0表示不是循环定时器,反之)
		 */
		Timer(uint32_t ms,std::function<void()> func,uint32_t interval = 0);

		Timer(timespec beginTime,std::function<void()>,timespec intervalTime);

		Timer(uint32_t beginTime,Fiber fiber,uint32_t intervalTime);

		/**
		 * @brief getFunc 获取定时器执行函数
		 *
		 * @return function
		 */
		std::function<void()> getFunc();

		//static void runFunc(int signo,Timer::ptr timer);

	private:
		//用途 比较 Time   作为set排序的依据
		struct Comparator{
			bool operator()(const Timer::ptr & t1,const Timer::ptr & t2);
		};


	};

	class TimerManager{
	public:
		typedef std::shared_ptr<TimerManager> ptr;

		TimerManager();
		virtual ~TimerManager();

		static TimerManager* getTimerManager();

		/**
		 * @brief addTimer 增加定时器
		 *	
		 * @param ms	定时器的周期
		 * @param func	定时器执行方法
		 * @param recurring	是否是循环计时器
		 *
		 * @return 计时器对象
		 */
		Timer::ptr addTimer(uint32_t ms,std::function<void()> func,
						uint32_t interval);

		void addTimer(timespec span,std::function<void()> func,timespec interval);


		/**
		 * @brief delTimer 删除定时器
		 *
		 * @param timer 定时器对象(智能指针)
		 *
		 * @return 返回删除是否成功
		 */
		bool delTimer(Timer::ptr & timer);

		/**
		 * @brief addConditionTimer 条件定时器
		 * 当条件还在的时候才会触发定时器
		 * @param ms	时间间隔
		 * @param func	触发函数
		 * @param weak_cond	使用weak_ptr 来做条件变量
		 * @param recurring	循环定时器
		 *
		 * @return 定时器
		 */
		Timer::ptr addConditionTimer(uint32_t ms,std::function<void()> func,
					std::weak_ptr<void> weak_cond,bool recurring = false);
		
		/**
		 * @brief getNextExecTime 获取下一个执行的时间
		 *
		 * @return 下一个执行的时间
		 */
		uint32_t getNextExecTime();

		/**
		 * @brief listExpiredTimer 就会过期的定时器
		 *
		 * @param timers	返回参数
		 */
		//void listExpiredTimer(std::vector<Timer::ptr>& timers);

		void runTimer();
	protected:
		//定时器接口
		int m_timerFd;
		//z自定义类型的排序默认使用地址进行排序,
		//comparator 使用自定的排序方式
		std::vector<Timer::ptr> m_timers;
	private:
	
		RWMutex m_rwMutex;

	};
}

#endif

