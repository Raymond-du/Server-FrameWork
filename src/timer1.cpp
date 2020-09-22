
#include "timer1.h"
#include "RWMutex.h"
#include "iomanager.h"
#include "schedule.h"
#include "util.h"
#include <algorithm>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <bits/types/struct_itimerspec.h>
#include <bits/types/struct_timeval.h>
#include <csignal>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/select.h>
#include <unistd.h>
#include <vector>
#include "macro.h"
#include <sys/timerfd.h>


namespace raymond{

	static TimerManager::ptr g_timerManager = nullptr;
	static Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");

	Timer::Timer(uint32_t ms,std::function<void()>func,
				uint32_t interval):
			m_func(func){

		uint32_t sec = ms/1000;
		m_triggerTime.tv_sec = sec;
		m_triggerTime.tv_nsec = (ms - sec*1000) * 1000 * 1000;
		
		if(interval){
			sec = interval/1000;
			m_intervalTime.tv_sec = sec;
			m_intervalTime.tv_nsec = (ms - sec*1000) * 1000 * 1000;
		}else{
			m_intervalTime.tv_sec = 0;
			m_intervalTime.tv_nsec = 0;
		}
		
		timeval tv = getCurTime();
		m_execTime.tv_sec = tv.tv_sec + m_triggerTime.tv_sec;
		m_execTime.tv_nsec = tv.tv_usec * 1000 + m_triggerTime.tv_nsec;
	}

	Timer::Timer(timespec span, std::function<void ()> func, timespec interval):
			m_func(func){
	

		m_triggerTime = span;
		m_intervalTime = interval;
		
		timeval tv = getCurTime();
		m_execTime.tv_sec = tv.tv_sec + span.tv_sec;
		m_execTime.tv_nsec = tv.tv_usec * 1000 + span.tv_nsec;
	}

	bool Timer::isRecurring(){
		return m_intervalTime.tv_sec != 0 && 
			m_intervalTime.tv_nsec != 0;
	}

	std::function<void()> Timer::getFunc(){
		return m_func;
	}

	bool Timer::Comparator::operator()(const Timer::ptr & t1,
									const Timer::ptr & t2){
		if(!t1 && !t2){
			return false;
		}
		if(!t1){
			return false;
		}
		if(!t2){
			return true;
		}
		if(t1->m_execTime.tv_sec > t2->m_execTime.tv_sec){
			return true;
		}
		if(t1->m_execTime.tv_nsec > t2->m_execTime.tv_nsec){
			return true;
		}
		return false;
	}

	TimerManager::TimerManager(){
		m_timerFd = timerfd_create(CLOCK_REALTIME,0);
		RAYMOND_ASSERT2(m_timerFd != -1,"timerfd_create error");
	}

	TimerManager* TimerManager::getTimerManager(){
		return dynamic_cast<TimerManager*>(IOManager::getIOManager());
	}

	TimerManager::~TimerManager(){
		close(m_timerFd);
	}

	Timer::ptr TimerManager::addTimer(uint32_t ms,std::function<void()> func,
						uint32_t interval){
		Timer::ptr timer(new Timer(ms,func,interval));
		bool isAtFront = false;
		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			//添加timer到victor中 进行最小堆的格式化.			
			m_timers.push_back(timer);
			std::push_heap(m_timers.begin(),m_timers.end(),Timer::Comparator());
			
			isAtFront = (timer.get() == m_timers.begin()->get()); 
		}
		if(isAtFront){
			//执行timer 将接下来的timer 加入定时器
			itimerspec  span;
			span.it_value.tv_sec = timer->m_triggerTime.tv_sec;
			span.it_value.tv_nsec = timer->m_triggerTime.tv_nsec;
			span.it_interval.tv_nsec = 0;
			span.it_interval.tv_sec = 0;
			int rt = timerfd_settime(m_timerFd,0,&span,nullptr);
			RAYMOND_ASSERT2(rt == 0,"timerfd_settimer error ");
		}
		RAYMOND_LOG_FMT_DEBUG(g_logger,"addtimer success !!");
		return timer;
	}

	bool TimerManager::delTimer(Timer::ptr & timer){
		std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
		
		for(auto it = m_timers.begin();it != m_timers.end();++it){
			if((*it).get() == timer.get()){
				m_timers.erase(it);
				return true;
			}
		}
		return false;	
	}

	static void onTimer(std::weak_ptr<void> weak_cond,std::function<void()> func){
		//如果weak_cond 没有被释放则 执行func函数
		auto tmp = weak_cond.lock();
		if(tmp){
			func();
		}
	}

	Timer::ptr TimerManager::addConditionTimer(uint32_t ms,std::function<void()> func,
				std::weak_ptr<void> weak_cond,bool recurring){
		
		return addTimer(ms,std::bind(onTimer,weak_cond,func),recurring);
	}

	uint32_t TimerManager::getNextExecTime(){
		std::unique_lock<RWMutex::RLock> lock(m_rwMutex);
		if(m_timers.empty()){
			//返回一个最大值
			return ~0;
		}
		auto timer = *m_timers.begin();
		auto nowTime = getCurTime();
		uint32_t timerMs = timer->m_execTime.tv_sec * 1000 +
				timer->m_execTime.tv_nsec / 1000 / 1000;
		uint32_t nowMs = nowTime.tv_sec * 1000 + nowTime.tv_usec / 1000;
		if(timerMs > nowMs){
			return timerMs - nowMs;
		}
		return 0;
	}

	void TimerManager::runTimer(){
		Timer::ptr timer = nullptr;
		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			//获取 第一个timer 并将他删除
			timer = *m_timers.begin();
			std::pop_heap(m_timers.begin(),m_timers.end(),Timer::Comparator());
			m_timers.pop_back();
		}

		timer->getFunc()();
	
		if(timer->isRecurring()){
			addTimer(timer->m_intervalTime,timer->getFunc(),timer->m_intervalTime);
		}
		
		uint32_t next = 0;
		while((next = getNextExecTime()) == 0){
			runTimer();
		}	

		{
			std::unique_lock<RWMutex::RLock> lock(m_rwMutex);
			if(m_timers.empty()){
				return ;
			}
			timer = *m_timers.begin();
			timeval now_time = getCurTime();
			itimerspec ts ;
			long _nsec = timer->m_execTime.tv_nsec - now_time.tv_usec * 1000;
			ts.it_value.tv_sec = timer->m_execTime.tv_sec - now_time.tv_sec;
			if(_nsec < 0){
					ts.it_value.tv_sec -= 1;
					ts.it_value.tv_nsec = 1 * 1000 * 1000 * 1000 + _nsec;
			}else{
				ts.it_value.tv_nsec = _nsec;

			}
			timerfd_settime(m_timerFd,0,&ts,nullptr);
		}
			
	}

	void TimerManager::addTimer(timespec span,std::function<void()>func,timespec interval){
		
		Timer::ptr timer(new Timer(span,func,interval));
		bool isAtFront = false;
		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			//添加timer到victor中 进行最小堆的格式化.			
			m_timers.push_back(timer);
			std::push_heap(m_timers.begin(),m_timers.end(),Timer::Comparator());
			
			isAtFront = (timer.get() == m_timers.begin()->get()); 
		}
		if(isAtFront){
			//执行timer 将接下来的timer 加入定时器
			itimerspec ts;
			ts.it_value.tv_sec = timer->m_triggerTime.tv_sec;
			ts.it_value.tv_nsec = timer->m_triggerTime.tv_nsec;

			timerfd_settime(m_timerFd,0,&ts,nullptr);
		}

		return;
	
	}
}

