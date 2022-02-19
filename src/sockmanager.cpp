#include "sockmanager.h"
#include <algorithm>
#include <fcntl.h>
#include <mutex>
#include <pthread.h>
#include <string>
#include <sys/stat.h>
#include "Logger.h"
#include "RWMutex.h"
#include "hook.h"
#include "util.h"
#include "socketImpl.h"

namespace raymond{

	static auto g_logger = RAYMOND_LOG_BYNAME("system");
//	SockContext::SockContext(int fd)
//					:m_fd(fd)
//					,m_flag(0x00)
//					,m_recvTimeOut(0)
//					,m_sendTimeOut(0) {
//		struct stat fd_stat;
//		if(-1 == fstat(fd,&fd_stat)){
//			setInit(false);
//			setSock(false);
//		}else{
//			setInit(true);
//			if(S_ISSOCK(fd_stat.st_mode)){
//				setSock(true);
//			}else{
//				setSock(false);
//			}
//		}
//		if(isSocket()){
//			//获取文件描述的信息	
//			int flags = fcntl_f(fd,F_GETFL,0);
//			if(flags & O_NONBLOCK){
//				setNonblock(true);
//			} else {
//				setNonblock(false);
//			}
//		}
//		setClose(false);
//	}
//	
//	bool SockContext::isInit(){
//		return m_flag & 0x01;
//	}
//
//	bool SockContext::isSocket(){
//		return m_flag & 0x02;
//	}
//	
//	void SockContext::setNonblock(bool v){
//		m_flag = v ? (m_flag | 0x04) : (m_flag & ~0x04);
//	}
//
//	bool SockContext::isNonblock(){
//		return m_flag & 0x04;
//	}
//
//	bool SockContext::isClose(){
//		return m_flag & 0x08;
//	}
//	
//	void SockContext::setTimeOut(SockType type,uint32_t t){
//		if(type == RECV){
//			m_recvTimeOut = t;
//		}else{
//			m_sendTimeOut = t;
//		}
//	}
//
//	uint32_t SockContext::getTimeOut(SockType type){
//		return type == RECV ? m_recvTimeOut : m_sendTimeOut;
//	}
//	
//	void SockContext::setInit(bool v){
//		m_flag = v ? m_flag | 0x01 : m_flag & ~0x01;
//	}
//	
//	void SockContext::setSock(bool v){
//		m_flag = v ? m_flag | 0x02 : m_flag & ~0x02;
//	}
//
//	void SockContext::setClose(bool v){
//		m_flag = v ? m_flag | 0x08 : m_flag & ~0x08;
//	}

	bool SockManager::addSock(SocketImpl::ptr sock) {
		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			auto it = m_socks.find(sock->getSock());
			if (it != m_socks.end()) {
				return false;
			}
		}
		{
			std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
			m_socks.insert(std::make_pair(sock->getSock(), sock));
		}
		return true;
	}

	SocketImpl::ptr SockManager::getSock(int fd){
		SocketImpl::ptr sockCtx = nullptr;
		{
			std::unique_lock<RWMutex::RLock> lock(m_rwMutex);
			auto it = m_socks.find(fd);
			if(it != m_socks.end()) {
				return it->second;
			}
		}
		return sockCtx;
	}
	
	bool SockManager::delSock(int fd){
		RAYMOND_LOG_FMT_DEBUG(g_logger, "sockmanager删除socket:%d", fd);
		std::unique_lock<RWMutex::WLock> lock(m_rwMutex);
		auto it = m_socks.find(fd);
		if(it != m_socks.end()){
			m_socks.erase(it);
			return true;
		}
		return false;
	}

	bool SockManager::delSock(SocketImpl::ptr sock) {
		return delSock(sock->getSock());
	}

}
