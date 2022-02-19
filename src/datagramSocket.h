#ifndef __DATAGRAMSOCKET_H
#define __DATAGRAMSOCKET_H

#include "address.h"
#include "noncopy.h"
#include "socketImpl.h"
#include <cstddef>
#include <memory>
#include <sys/types.h>
namespace raymond{
	class DatagramSocket : public SocketImpl, NonCopyable{
	public:
		typedef std::shared_ptr<DatagramSocket> ptr;
		DatagramSocket(int domain, bool hookEnable = false);

		static DatagramSocket::ptr Create(Address::ptr addr);

		bool Bind(Address::ptr Address);
		ssize_t RecvFrom(void *buf,size_t len,Address::ptr src_addr);
		ssize_t	Sendto(void *buf,size_t len,const Address::ptr dest_addr);
		bool Close() override;

		bool setNonBlock(bool nonBlock);
	private:
		int m_family = 0;
		bool m_bound = false;
		bool m_hookEnable = false;
		bool m_nonBlock = false;
		Address::ptr m_bindAddr;

	};
}

#endif
