#include "datagramSocket.h"
#include "Logger.h"
#include "address.h"
#include "sockmanager.h"
#include <cstddef>
#include <fcntl.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>

namespace raymond {
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	DatagramSocket::DatagramSocket(int domain, bool hookEnable) {
		m_hookEnable = hookEnable;
		m_family = domain;
		m_sock = socket(domain, SOCK_DGRAM, 0);
		
		if (hookEnable) {
			auto sockCtx = sockMgr::getInstance()->get(m_sock, true);
			sockCtx->setTimeOut(SockContext::RECV, -1);
			sockCtx->setTimeOut(SockContext::SEND, -1);
		}
	}

	DatagramSocket::ptr DatagramSocket::Create(Address::ptr addr) {
		DatagramSocket::ptr sock(new DatagramSocket(addr->getFamily()));
		sock->Bind(addr);
		return sock;
	}

	bool DatagramSocket::Bind(Address::ptr addr) {
		if (addr->getFamily() != m_family) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket's family %d is "\
					"different with bind family %d",m_family,addr->getFamily());
			return false;
		}
		if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"socket %d bind %s error : %s",
					m_sock,addr->toString().c_str(),std::strerror(errno));
			return false;
		}
		m_bindAddr = addr;
		m_bound = true;
		return true;
	}

	ssize_t DatagramSocket::RecvFrom(void *buf,size_t len,Address::ptr src_addr) {
		sockaddr sa;
		socklen_t saLen = sizeof(sa);
		ssize_t res = ::recvfrom(m_sock, buf, len, 0, &sa, &saLen);
		src_addr = Address::createAddr(&sa, saLen);
		return res;
	}

	ssize_t	DatagramSocket::Sendto(void *buf,size_t len,const Address::ptr dest_addr) {
		return ::sendto(m_sock, buf, len, 0, dest_addr->getAddr(), dest_addr->getAddrLen());
	}

	bool DatagramSocket::Close() {
		if (m_sock != -1) {
			::close(m_sock);
			m_sock = -1;
		}
		return true;
	}

	bool DatagramSocket::setNonBlock(bool nonBlock) {
		if (m_nonBlock == nonBlock) {
			return true;
		}
		int flags = 0;
		int rt = fcntl(m_sock, F_GETFL, &flags, sizeof(int));
		if (rt == -1) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"fcntl GETFD %d error : %s",
					m_sock,std::strerror(errno));
			return false;		
		}
		if (nonBlock == true) {
			flags |= O_NONBLOCK;
		} else {
			flags &= ~O_NONBLOCK;
		}

		rt = fcntl(m_sock, F_SETFL, flags, sizeof(int));
		if (rt == -1) {
			RAYMOND_LOG_FMT_ERROR(g_logger,"fcntl GETFD %d error : %s",
					m_sock,std::strerror(errno));
			return false;
		}
		m_nonBlock = nonBlock;
		return true;
	}
}
