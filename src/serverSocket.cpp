#include "serverSocket.h"
#include "address.h"
#include "socket.h"
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include "iomanager.h"
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
#include "Logger.h"
#include "sockmanager.h"

namespace raymond{
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	ServerSocket::ServerSocket(int domain) {
		m_family = domain;
		m_sock = socket(domain,SOCK_STREAM,0);
		if (m_sock != -1) {
			m_valid = true;
		}

		//setHookEnable(hookEnable);
		//if(hookEnable == true) {
		//	auto sockCtx = sockMgr::getInstance()->get(m_sock, true);
		//	sockCtx->setTimeOut(SockContext::RECV, -1);
		//	sockCtx->setTimeOut(SockContext::SEND, -1);
		//}
	}

	ServerSocket::~ServerSocket(){
		Close();
	}

	ServerSocket::ptr ServerSocket::Create(Address::ptr addr){
		ServerSocket::ptr sock(new ServerSocket(addr->getFamily()));
		
		sock->Bind(addr);
		return sock;
	}

	bool ServerSocket::Bind(Address::ptr addr){
		if(addr->getFamily() != m_family){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket's family %d is "\
					"different with bind family %d",m_family,addr->getFamily());
			return false;
		}
		if(::bind(m_sock,addr->getAddr(),addr->getAddrLen())){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d bind %s error : %s",
					m_sock,addr->toString().c_str(),std::strerror(errno));
			return false;
		}
		m_bindAddr = addr;
		m_bound = true;
		return true;
	}
		
	bool ServerSocket::Listen(int backlog){
		if(::listen(m_sock,backlog)){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d Listen %s error : %s",m_sock,std::strerror(errno));
			return false;
		}
		return true;
	}

	Socket::ptr ServerSocket::Accept(){
		char addr[64] = {0};
		memset(addr,'\0',sizeof(addr));
		socklen_t addrLen = sizeof(addr);
		int c_sock = accept(m_sock,(sockaddr*)addr,&addrLen);
		if(c_sock == -1){
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d accept error : %s",
						m_sock,std::strerror(errno));
			return nullptr;
		}

		Address::ptr pAddr = Address::createAddr((sockaddr*)addr,addrLen);
		Socket::ptr c_socket;
		if (m_clientAutoConfig) {
			c_socket.reset(new Socket(c_sock, pAddr));
			c_socket->setNonBlock(m_nonBlock);
			c_socket->setHookEnable(m_hookEnable);
		} else {
			c_socket.reset(new Socket(c_sock, pAddr));
		}
		
		return c_socket;
	}
		
	bool ServerSocket::Close() {
		if(m_sock != -1){
			::close(m_sock);
			m_sock = -1;
		}
		return true;
	}
		
	void ServerSocket::setListenFun(std::function<void(ServerSocket::ptr)> func) {
		m_listenFunc = func;
		auto iomanager = IOManager::getIOManager();
		iomanager->addEvent(this->shared_from_this(), IOManager::READ, 
											std::bind(&ServerSocket::readSignal, this), true);
		RAYMOND_LOG_FMT_DEBUG(g_logger, "socket: %d, setListenerfunc", m_sock);
	}
	//bool ServerSocket::addAcceptEvent(CALLBACK callback, uint32_t timeout) {
	//	auto io = IOManager::getIOManager();
	//	if (io == nullptr) {
	//		return false;
	//	}
	//	std::shared_ptr<int> t_con(new int);
	//	std::weak_ptr<int> w_con(t_con);
	//	if (timeout != (uint32_t)-1) {
	//		io->addConditionTimer(timeout, [io, this](){
	//					io->delEvent(this->shared_from_this(), IOManager::READ);
	//				},w_con,false);
	//	}
	//	io->addEvent(this->shared_from_this(), IOManager::Event::READ, 
	//				std::bind(&ServerSocket::acceptEvent, this, t_con, callback), true);
	//	return true;
	//}

	//void ServerSocket::acceptEvent(std::shared_ptr<void> con, CALLBACK callback) {
	//	con.reset();
	//	Socket::ptr sock = Accept();
	//	callback(sock);
	//}
}
