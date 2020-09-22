
#include "iomanager.h"
#include "Logger.h"
#include "RWMutex.h"
#include "fiber.h"
#include "schedule.h"
#include <algorithm>
#include <asm-generic/errno-base.h>
#include <bits/stdint-uintn.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <sys/epoll.h>
#include "macro.h"
#include "thread.h"
#include "timer1.h"
#include "util.h"
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

namespace raymond{

	static Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");

	std::string IOManager::toString(IOManager::Event event){
		switch(event){
		case READ:
			return "read";
		case WRITE:
			return "write";
		default:
			return "none";
		}
	}

	void IOManager::FdContext::EventContext::reset(){
		m_schedule.reset();
		m_func = nullptr;
	}

	void IOManager::FdContext::triggerEvent(IOManager::Event event){
		EventContext* eCon = getContext(event);
		
		eCon->m_schedule->schedule(eCon->m_func);
	}

	IOManager::FdContext::EventContext* IOManager::FdContext::getContext(IOManager::Event event){
		switch(event){
		case READ:
			return &m_readEvent;
		case WRITE:
			return &m_writeEvent;
		default:
			return nullptr;

		}
	}

	IOManager::IOManager(const std::string & name,bool useCurThread,size_t coreNum):
			FiberSchedule(name,useCurThread,coreNum),
			TimerManager(){
		//初始化epoll 设置最大的监听fd的大小
		m_epollFd = epoll_create(5000);
		RAYMOND_ASSERT2(m_epollFd > 0,"epoll_create error");

		int rt = pipe(m_pipeFd);
		RAYMOND_ASSERT(!rt);
		
		//pipe[0]的读事件添加到epoll中
		epoll_event e_event;
		//边沿触发 读事件
		memset(&e_event,0,sizeof(e_event));
		e_event.events = EPOLLET | EPOLLIN;
		e_event.data.fd = m_pipeFd[0];
		//设置pipe[0]为非阻塞式的
		rt = fcntl(m_pipeFd[0],F_SETFL,O_NONBLOCK);
		RAYMOND_ASSERT(!rt);
	
		rt = epoll_ctl(m_epollFd,EPOLL_CTL_ADD,m_pipeFd[0],&e_event);
		RAYMOND_ASSERT(!rt);
		
		//监听定时器
		epoll_event timer_event;
		memset(&timer_event,0,sizeof(timer_event));
		timer_event.events = EPOLLET | EPOLLIN;
		timer_event.data.fd = m_timerFd;
		rt = epoll_ctl(m_epollFd,EPOLL_CTL_ADD,m_timerFd,&timer_event);
		RAYMOND_ASSERT(!rt);

		//添加waitfunc 到调度器中
		FiberSchedule::schedule(std::bind(&IOManager::waitFunc,this));
	}

	IOManager::~IOManager(){
			//join();
		//主要进行关闭pipe 等连接
		if(m_joinFlag == false){
			join();
		}
		close(m_epollFd);
		close(m_pipeFd[0]);
		close(m_pipeFd[1]);
	}

	bool IOManager::addEvent(int fd,Event event,std::function<void()> func){
		//先判断是否存在该事件
		auto fdCont = getFdContext(fd);

		if(!fdCont){
			fdCont.reset(new FdContext());
			fdCont->m_fd = fd;
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			m_fdsMap[fd] = fdCont;
		}
		{
			std::unique_lock<std::mutex> lock(fdCont->m_mutex);
		
			int op = fdCont->m_curEvent ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
			fdCont->m_curEvent = (Event)(fdCont->m_curEvent | event);

			epoll_event e_event;
			e_event.data.fd = fd;
			e_event.data.ptr = fdCont.get();
			e_event.events = fdCont->m_curEvent | EPOLLET;

			if(0 != epoll_ctl(m_epollFd,op,fd,&e_event)){
				RAYMOND_LOG_FMT_ERROR(g_logger,"epoll_ctl has error--%s",strerror(errno));
				return false;
			}
			
			++m_eventCount;
			//添加func到 fdCont中
			FdContext::EventContext* eCont = fdCont->getContext(event);
			eCont->m_schedule = this->shared_from_this();
			eCont->m_func.swap(func);

			RAYMOND_LOG_FMT_DEBUG(g_logger,"添加事件");

		}
		return true;
	}

	bool IOManager::delEvent(int fd,Event event,std::function<void()>func){
		FdContext::ptr fdCon = getFdContext(fd);
	
		if(!fdCon){
			RAYMOND_LOG_FMT_WARN(g_logger,"fd %s does't no exist",fd);
		}
		
		{
			std::unique_lock<std::mutex> lock(fdCon->m_mutex);
			if(RAYMOND_UNLIKELY(!(fdCon->m_curEvent & event))){
				RAYMOND_LOG_FMT_WARN(g_logger,"fd %d %s does't enroll",fd,toString(event).c_str());
				return false;
			}
				
			//epoll 监听事件中删除
			fdCon->m_curEvent = (Event)(fdCon->m_curEvent & (~event));
			int op = fdCon->m_curEvent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
			epoll_event e_event;
			e_event.events = fdCon->m_curEvent | EPOLLET;
			e_event.data.ptr = fdCon.get();

			if(0 != epoll_ctl(m_epollFd,op,fd,&e_event)){
				RAYMOND_LOG_FMT_ERROR(g_logger,"epoll_ctl has error--%s",strerror(errno));
				return false;
			}
			
			--m_eventCount;
			//如果读写事件都被删除了则吧 fd也给删除了
			if(op == EPOLL_CTL_DEL){
				std::unique_lock<RWMutex::WLock> lock2(m_rwMutex);
				m_fdsMap.erase(fd);
				//启动回调函数 通知 已删除对fd的监听
				if(func){
					func();
				}
			}else{
				FdContext::EventContext* eCon = fdCon->getContext(event);
				eCon->reset();
			}
		}

		return true;
	}
	
	bool IOManager::delFdEvent(int fd){
		auto fdCon = getFdContext(fd);

		if(RAYMOND_UNLIKELY(!fdCon)){
			RAYMOND_LOG_FMT_WARN(g_logger,"fd %d does't no exist",fd);
			return false;
		}
		{
			std::unique_lock<std::mutex> lock(fdCon->m_mutex);
			epoll_event e_event;
			e_event.events = 0;
			e_event.data.ptr = fdCon.get();
			if(0 != epoll_ctl(m_epollFd,EPOLL_CTL_DEL,fd,&e_event)){
				RAYMOND_LOG_FMT_ERROR(g_logger,"epoll_ctl has error--%s",strerror(errno));
				return false;
			}
			
			if(fdCon->m_curEvent & READ){
				--m_eventCount;
			}
			if(fdCon->m_curEvent & WRITE){
				--m_eventCount;
			}
		}

		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			m_fdsMap.erase(fd);
		}
		return true;
	}

	IOManager::FdContext::ptr IOManager::getFdContext(int fd){
		std::unique_lock<RWMutex::RLock> lock(m_rwMutex);
		auto it = m_fdsMap.find(fd);
		if(it == m_fdsMap.end()){
			return nullptr;
		}else{
			return it->second;
		}
	}
	
	IOManager* IOManager::getIOManager(){
		return dynamic_cast<IOManager*>(FiberSchedule::getScheduler());
	}
	
	

	//这个协程不会退出需要修改
	void IOManager::waitFunc(){
		RAYMOND_LOG_FMT_INFO(g_logger,"wait for message");
		//epoll最大的获取数量
		const uint32_t MAX_EVENT_SIZE = 256;
		epoll_event * e_events = new epoll_event[MAX_EVENT_SIZE];
		
		while(true){
			//判断 是否可以停止
			if(stopping()){
				RAYMOND_LOG_FMT_DEBUG(g_logger,"iomanager quit");
				break;
			}

			int rt = 0;
			do{
				rt = epoll_wait(m_epollFd,e_events,MAX_EVENT_SIZE,1000);
				RAYMOND_LOG_FMT_DEBUG(g_logger,"epoll _wait is %d",rt);
				if(rt < 0){
					if(errno == EINTR){
						continue;
					}else{
						RAYMOND_LOG_FMT_ERROR(g_logger,"epoll_wait error,error reason is %s",strerror(errno));
						return ;
					}
				}else{
					break;
				}
			}while(true);
			//执行 相关的 读写操作
			for(size_t i = 0;i < rt;i++){
				//如果是 pipe的读事件可以进行 关闭等其他操作
				if(e_events[i].data.fd == m_pipeFd[0]){
					//这个地方是否必须要读完?
					char buf = 0;
					read(m_pipeFd[0],&buf,1);
					switch(buf){
					case 'Q':
						return ;
					}
					continue;
				}
				if(e_events[i].data.fd == m_timerFd){
					FiberSchedule::schedule(std::bind(&TimerManager::runTimer,this));
					continue;
				}

				FdContext * fdCon = (FdContext*)(e_events[i].data.ptr);
				if(fdCon == nullptr){
					continue;
				}
				//如果监听的事件出现错误 执行可执行的操作
				if(e_events[i].events & (EPOLLERR | EPOLLHUP)){
					e_events[i].events |= (EPOLLIN | EPOLLOUT) & fdCon->m_curEvent;
				}

				int real_event = e_events[i].events;
				
				if((real_event & fdCon->m_curEvent)==NONE){
					continue;
				}else{
					if(real_event & READ){
						fdCon->triggerEvent(READ);
					}
					if(real_event & WRITE){
						fdCon->triggerEvent(WRITE);
					}
				}
			}
		}

		delete []e_events;
	}

	bool IOManager::stopping(){
		//判断是否可以结束 定时器没有任务 
		return m_timers.empty() && m_joinFlag == true && m_fibers.empty() && m_fibers.empty();
		
	}

	void IOManager::join(){
		FiberSchedule::join();
	}
}
