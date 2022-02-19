#include "socketImpl.h"
#include <cstring>
//#include <fcntl.h>
#include "hook.h"
#include <sys/socket.h>
#include "Logger.h"
#include "iomanager.h"
#include "sockmanager.h"

namespace raymond{
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	SocketImpl::~SocketImpl() {
		if (m_sock > 0) {
			Close();
		}
	}

	bool SocketImpl::setOpt(int level,int option,const void *result,socklen_t len){
		int rt = setsockopt(m_sock,level,option,result,len);
		if(rt == -1){
			RAYMOND_LOG_FMT_ERROR(g_logger,"setsockopt %d error : %s",
						m_sock,std::strerror(errno));
			return false;
		}
		return true;
	}

	bool SocketImpl::getOpt(int level,int option,void *result,socklen_t* len){
		int rt = getsockopt(m_sock,level,option,result,len);
		if(rt == -1){
			RAYMOND_LOG_FMT_ERROR(g_logger,"getsockopt %d error : %s",
						m_sock,std::strerror(errno));
			return false;
		}
		return true;
	}

	int SocketImpl::getSock() const{
		return m_sock;
	}

	void SocketImpl::setSendTimeout(uint32_t t){
		timeval tv{int(t / 1000),int(t % 1000 * 1000)};
		setOpt(SOL_SOCKET,SO_SNDTIMEO,tv);
	}

	uint32_t SocketImpl::getSendTimeout(){
		return m_sendTimeOut;
	}

	void SocketImpl::setRecvTimeout(uint32_t t){
		timeval tv{int(t / 1000),int(t % 1000 * 1000)};
		setOpt(SOL_SOCKET,SO_RCVTIMEO,tv);
	}

	uint32_t SocketImpl::getRecvTimeout(){
		return m_recvTimeOut;
	}

	bool SocketImpl::setHookEnable(bool v) {
		bool rt = false;
		if (v == true) {
			m_hookEnable = true;
			rt = sockMgr::getInstance()->addSock(this->shared_from_this());
		} else {
			m_hookEnable = false;
			rt = sockMgr::getInstance()->delSock(m_sock);
			IOManager::getIOManager()->delFdEvent(this->shared_from_this());
		}
		return rt;
	}

	bool SocketImpl::setNonBlock(bool v) {
		if (m_nonBlock == v) {
			return true;
		} 
		int flags = 0;
		flags = fcntl_f(m_sock,F_GETFL);
		flags = v ? flags | O_NONBLOCK : flags & ~O_NONBLOCK;
		int rt = fcntl_f(m_sock, F_SETFL, flags);
		if (rt == -1) {
			RAYMOND_LOG_FMT_WARN(g_logger, "socket:%d, setNonBlock(%s) error :%d", m_sock, v ? "true" : "false", errno);
			return false;
		}
		m_nonBlock = v;
		return true;
	}

	bool SocketImpl::Close() {
		if (m_sock != -1 && m_valid == true) {
			::close(m_sock);
			m_sock = -1;
			m_valid = false;
			return true;
		} else {
			return false;
		}
	}
}
