#include "socket.h"
#include "Logger.h"
#include "address.h"
#include "iomanager.h"
#include "socketImpl.h"
#include "sockmanager.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <bits/stdint-uintn.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include "macro.h"
#include "hook.h"

namespace raymond{
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	Socket::Socket(int domain){
		m_family = domain;
		m_sock = ::socket(domain,SOCK_STREAM,0);
		if(m_sock == -1){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket create error : %s",
				std::strerror(errno));
			m_valid = false;
			return ;
		}
		m_valid = true;
		initSock();
	}

	Socket::Socket(int sock, const Address::ptr& addr)
									:SocketImpl(sock)
									,m_connected(true) {
		m_valid = true;
		//if (hookEnable) {
		//	sockMgr::getInstance()->addSock(this->shared_from_this());

		//	sockCtx->setNonblock(nonBlock);
		//	sockCtx->setTimeOut(SockContext::RECV, m_recvTimeOut);
		//	sockCtx->setTimeOut(SockContext::SEND, m_sendTimeOut);
		//}
		m_destAddr = addr;
	}

	Socket::~Socket(){
		Close();
		RAYMOND_LOG_FMT_DEBUG(g_logger, "socket:%d 析构", m_sock);
	}

	void Socket::initSock(){
		int val = 1;
		setOpt(SOL_SOCKET,SO_REUSEADDR,val);
		setOpt(IPPROTO_TCP,TCP_NODELAY,val);
	}

	bool Socket::Bind(const Address::ptr & addr){
		if(addr->getFamily() != m_family){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket's family %d is "\
					"different with bind family %d",m_family,addr->getFamily());
			return false;
		}

		if(-1 == ::bind(m_sock,addr->getAddr(),addr->getAddrLen())){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d bind %s error : %s",
					m_sock,addr->toString().c_str(),std::strerror(errno));
			return false;
		}
		m_bindAddr = addr;
		m_bound = true;
		return true;
	}

	bool Socket::Connect(const Address::ptr addr,int timeout){
		if(RAYMOND_UNLIKELY(!m_valid)){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d is invalid",m_sock);
			return false;
		}
		
		if(connect_with_timeout(m_sock,addr->getAddr(),addr->getAddrLen(),timeout)){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d connect with %s error %s",
					m_sock,addr->toString().c_str(),std::strerror(errno));
			return false;
		}	
		
		if(!m_bindAddr){
			char sa[64];
			socklen_t sa_len = sizeof(sa);
			if(getsockname(m_sock,(sockaddr*)sa,&sa_len)){
				RAYMOND_LOG_FMT_ERROR(g_logger,"getsockname %d error : %s",
						m_sock,std::strerror(errno));
			}else{
				m_bindAddr = Address::createAddr((sockaddr*)sa,sa_len);
			}
		}
		m_connected = true;
		m_destAddr = addr;
		return true;
	}

	bool Socket::reConnect(int timeout) {
		if(!m_destAddr){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d reconnect error : " \
					"m_destAddr is null");
			return false;
		}
		return Connect(m_destAddr,timeout);
	}

	bool Socket::Close() {
		if (SocketImpl::Close() == true) {
			m_connected = false;
			return true;
		}
		return false;
	}

	//bool Socket::setNonBlock(bool nonBlock){
	//	if(m_nonBlock == nonBlock){
	//		return true;
	//	}
	//	int flags = 0;
	//	int rt = fcntl(m_sock,F_GETFL,&flags,sizeof(int));
	//	if(rt == -1){
	//		RAYMOND_LOG_FMT_ERROR(g_logger,"fcntl GETFD %d error : %s",
	//				m_sock,std::strerror(errno));
	//		return false;
	//	}
	//	if (nonBlock == true) {
	//		flags |= O_NONBLOCK;
	//	} else {
	//		flags &= ~O_NONBLOCK;
	//	}

	//	rt = fcntl(m_sock,F_SETFL, flags, sizeof(int));
	//	if(rt == -1){
	//		RAYMOND_LOG_FMT_ERROR(g_logger,"fcntl GETFD %d error : %s",
	//				m_sock,std::strerror(errno));
	//		return false;
	//	}
	//	m_nonBlock = nonBlock;
	//	return true;
	//}

	Socket::ptr Socket::CreateTCP(const Address::ptr &addr){
		Socket::ptr sock(new Socket(addr->getFamily()));
		sock->Bind(addr);
		return sock;
	}

    Socket::ptr Socket::CreateTCP(int domain){
		return Socket::ptr(new Socket(domain));
	}
    
	//void Socket::setSendTimeout(uint32_t t){
	//	timeval tv{int(t / 1000),int(t % 1000 * 1000)};
	//	setOpt(SOL_SOCKET,SO_SNDTIMEO,tv);
	//}

	//uint32_t Socket::getSendTimeout(){
	//	SockContext::ptr sockCtx = sockMgr::getInstance()->get(m_sock);
	//	if(sockCtx){
	//		return sockCtx->getTimeOut(SockContext::SockType::SEND);
	//	}
	//	timeval tv;
	//	getOpt(SOL_SOCKET,SO_SNDTIMEO,tv);
	//	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	//}

	//void Socket::setRecvTimeout(uint32_t t){
	//	timeval tv{int(t / 1000),int(t % 1000 * 1000)};
	//	setOpt(SOL_SOCKET,SO_RCVTIMEO,tv);
	//}

	//uint32_t Socket::getRecvTimeout(){
	//	SockContext::ptr sockCtx = sockMgr::getInstance()->get(m_sock);
	//	if(sockCtx){
	//		return sockCtx->getTimeOut(SockContext::SockType::RECV);
	//	}
	//	timeval tv;
	//	getOpt(SOL_SOCKET,SO_RCVTIMEO,tv);
	//	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	//}

	Address::ptr Socket::getLocalAddr(){
		if(m_bindAddr){
			return m_bindAddr;
		}
		char sa[64];
		socklen_t sa_len = sizeof(sa);
		if(getsockname(m_sock,(sockaddr*)sa,&sa_len)){
			RAYMOND_LOG_FMT_WARN(g_logger,"getsockname %d error : %s",
					m_sock,std::strerror(errno));
			return nullptr;
		}
		m_bindAddr = Address::createAddr((sockaddr*)sa,sa_len);
		return m_bindAddr;
	}

	Address::ptr Socket::getRemoteAddr(){
		if(m_destAddr){
			return m_destAddr;
		}
		char sa[64];
		socklen_t sa_len = sizeof(sa);
		if(getpeername(m_sock,(sockaddr*)sa,&sa_len)){
			RAYMOND_LOG_FMT_WARN(g_logger,"getpeername %d error : %s",
					m_sock,std::strerror(errno));
			return nullptr;
		}
		m_destAddr = Address::createAddr((sockaddr *)sa,sa_len);
		return m_destAddr;
	}

	bool Socket::isConnected(){
		return m_connected;
	}

	ssize_t Socket::Recv(void *buf,size_t len){
		ssize_t rt = ::recv(m_sock, buf, len, 0);
		if (rt <= 0) {
			Close();
		}
		return rt;
	}

	ssize_t Socket::Send(void *buf,size_t len){
		ssize_t rt =  ::send(m_sock,buf,len,0);	
		if (rt <= 0) {
			Close();
		}
		return rt;
	}

	bool Socket::setAsynReadEvent(AsynFunc cb) {
		m_readAsynFunc = cb;
		return IOManager::getIOManager()->addEvent(
									this->shared_from_this(),
									IOManager::Event::READ,
									std::bind(&Socket::readSignal, this),
									true);
	}

	//bool Socket::setAsynSendEvent() {
	//	return IOManager::getIOManager()->addEvent(
	//								this->shared_from_this(),
	//								IOManager::Event::WRITE,
	//								std::bind(&Socket::sendSignal, this),
	//								true);
	//}

	//void Socket::setHookEnable(bool v) {
	//	if (v) {
	//		sockMgr::getInstance()->addSock(this->shared_from_this());
	//	} else {
	//		sockMgr::getInstance()->delSock(m_sock);
	//	}
	//	m_hookEnable = v;
	//}
	
	void Socket::asynRead() {
		ssize_t len = 0;
		char buf[1024] = {0,};
		while (1) {
			ssize_t rt = recv_f(m_sock, buf, 1024, 0);
			if (rt > 0) {
				m_recvBuf.write(buf, rt);
			} else if (rt == 0) {
				Close();
				break;
			} else {
				//表示中断
				if (errno == EINTR) {
					continue;
				} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
					//表示内核缓冲区没有数据了
					break;
				}
			}
		}
		//如果接受缓冲区中存在数据  就需要通知处理缓冲区的数据
		while(m_recvBuf.getSize() > 0) {
			m_readAsynFunc(std::dynamic_pointer_cast<Socket>(this->shared_from_this()));
		}
	}
	void Socket::asynSend() {
		char buf[1024] = {0, };
		while (m_sendBuf.getSize() > 0) {
			size_t rt = m_sendBuf.read(buf, 1024);
			rt = send_f(m_sock, buf, rt, 0);
			if (rt == 0) {
				Close();
			} else {
				//表示中断
				if (errno == EINTR) {
					rt = write_f(m_sock, buf, rt);
				} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
					//表示内核缓冲区没有数据了
					break;
				}
			}
		}
	}
}
