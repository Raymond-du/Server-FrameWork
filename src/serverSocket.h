#ifndef __SERVERSOCKET_H
#define __SERVERSOCKET_H

#include "address.h"
#include "iomanager.h"
#include "noncopy.h"
#include "socketImpl.h"
#include "socket.h"
#include "timer1.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <list>
#include <memory>
#include <utility>

namespace raymond{
	class ServerSocket : public SocketImpl,
												public NonCopyable {
	public:
		/**
		 * @brief ServerSocket serversock的构造函数
		 *
		 * @param domain AF_INET 等协议
		 */
		ServerSocket(int domain);
		~ServerSocket();

		typedef std::shared_ptr<ServerSocket> ptr;

		/**
		 * @brief Create 创建一个tcpserver的socket
		 *	
		 * @param addr 绑定的address地址
		 * @param domain	协议族
		 * @param protocol	协议
		 *
		 * @return 
		 */
		static ServerSocket::ptr Create(Address::ptr addr);
		
		void readSignal() override { m_listenFunc(std::dynamic_pointer_cast<ServerSocket>(shared_from_this()));}
		void sendSignal() override {}
		
		bool Bind(Address::ptr addr);
		bool Listen(int backlog);
		Socket::ptr Accept();
		bool Close() override;

		Address::ptr getLocalAddr() { return m_bindAddr;}

		void setClientAutoConfig(bool v) { m_clientAutoConfig = v;}

		bool isClientAutoConfig() { return m_clientAutoConfig;}

		void setListenFun(std::function<void(ServerSocket::ptr)> func);
	//	typedef void (*CALLBACK) (Socket::ptr);
	//	/**
	//	 * @brief addAccpetEvent 添加accept事件
	//	 *
	//	 * @param callback 回调函数
	//	 * @param timeout 超时时间
	//	 * @return 添加是否成功
	//	 */
	//	bool addAcceptEvent(CALLBACK callback, uint32_t timeout = -1);
	
	protected:
	//	virtual void acceptEvent(std::shared_ptr<void> con, CALLBACK callback);

	private:
		int m_family = 0;
		//自动配置接收到的客户端的属性 初始化为true
		bool m_clientAutoConfig = true;
		Address::ptr m_bindAddr = nullptr;
		std::function<void(ServerSocket::ptr)> m_listenFunc;
	};
}
#endif
