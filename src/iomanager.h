
#ifndef __IOMANAGER_H
#define __IOMANAGER_H

#include "schedule.h"
#include "RWMutex.h"
#include "timer1.h"
#include <atomic>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace raymond{
	class IOManager : public FiberSchedule ,public TimerManager{
	public:

		typedef std::shared_ptr<IOManager> ptr;

		/**
		 * @brief 监听文件描述符的事件
		 */
		enum Event{
			NONE	= 0x00,
			READ	= 0x01, //=EPOLLIN
			WRITE	= 0x04,	//=EPOLLOUT
		};

		static std::string toString(Event event);
	protected:
		/**
		 * @brief 对文件描述符的封装
		 */
		struct FdContext{

			typedef std::shared_ptr<FdContext> ptr;
			/**
			 * @brief 事件的封装
			 */
			struct EventContext{
				/**
				 * @brief 当前事件的调度器
				 */
				FiberSchedule::ptr m_schedule;
				/**
				 * @brief 当前事件所要执行的方法
				 */
				std::function<void()> m_func;

				void reset();
			};

			//读事件
			EventContext m_readEvent;
			//写事件
			EventContext m_writeEvent;
			//文件描述符 socket
			int m_fd = 0;
			//当前监听的事件
			Event m_curEvent = NONE;
			//事件的mutex;
			std::mutex m_mutex;

			/**
			 * @brief getContext 获取对应的事件对象
			 *
			 * @param event 事件
			 *
			 * @return 事件对象的指针
			 */
			EventContext* getContext(Event event);

			/**
			 * @brief triggerEvent 调度事件该事件
			 *
			 * @param event 事件
			 */
			void triggerEvent(Event event);
		};	
		//epoll的句柄
		int m_epollFd = 0;
		//pipe管道的读写句柄 (负责通知epoll_wait,停止等待消息)
		int m_pipeFd[2] = {0};
		//记录事件的个数
		std::atomic_uint32_t m_eventCount = {0};
		//iomanage的读写锁
		RWMutex m_rwMutex;
		//文件描述符 和 事件的映射集合
		std::unordered_map<int,FdContext::ptr> m_fdsMap;

	protected:
		//void scheduleFunc() override;

		/**
		 * @brief waitFunc 等待epoll获得消息
		 */
		void waitFunc();

		/**
		 * @brief tickle 通知等待的
		 */
		//void notifyEvent();
		//
		bool stopping();

	public:
		IOManager(const std::string & name,bool useCurThread = false,size_t coreNum = 0);

		~IOManager();

		/**
		 * @brief addEvent 添加事件
		 *
		 * @param fd	文件描述符
		 * @param event	文件描述符的监听事件
		 * @param func	监听到事件后执行的func
		 *
		 * @return 添加是否成功
		 */
		bool addEvent(int fd,Event event,std::function<void()> func);

		/**
		 * @brief delEvent 删除指定描述符的监听事件
		 *	
		 * @param fd		指定的文件描述符
		 * @param event		文件描述符的事件
		 * @param func	如果对fd没有了监听事件触发该回调函数
		 *
		 * @return 返回是否成功
		 */
		bool delEvent(int fd,Event event,std::function<void()> func = nullptr);

		
		/**
		 * @brief delFdEvent 删除fd 中的所有事件
		 *
		 * @param fd 文件描述符
		 *
		 * @return 删除是否成功
		 */
		bool delFdEvent(int fd);

		/**
		 * @brief getFdContext 根据fd获取fdContext
		 *
		 * @param fd	文件描述符
		 *	
		 * @return  fdcontext
		 */
		FdContext::ptr getFdContext(int fd);

		/**
		 * @brief getIOManager 获取当前的调度器
		 *
		 * @return	iomanager的智能指针
		 */
		static IOManager* getIOManager(); 

		/**
		 * @brief stop 停止监听io
		 */
		void join();
	};
}


#endif
