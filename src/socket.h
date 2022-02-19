#ifndef __SOCKET_H
#define __SOCKET_H

#include "address.h"
#include "noncopy.h"
#include "socketImpl.h"
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>
#include <sys/socket.h>
#include <sys/types.h>

namespace raymond{
	class ServerSocket;
	/**
	 * @brief 客户端 socket
	 */
	class Socket :public NonCopyable, 
								public SocketImpl
								/*public std::enable_shared_from_this<Socket> */{
		friend ServerSocket;
	public:
		typedef std::shared_ptr<Socket> ptr;
		typedef std::function<void(Socket::ptr)> AsynFunc;
		Socket(int domain);

		virtual ~Socket();
	
		/**
		 * @brief CreateTCP 创建一个socket 绑定addr本地地址
		 *
		 * @param addr  绑定的地址
		 * @return socket 对象
		 */
		static Socket::ptr CreateTCP(const Address::ptr &addr);
		static Socket::ptr CreateTCP(int domain);

		/**
		 * @brief Bind 绑定本地地址
		 *
		 * @param addr 绑定的地址
		 *
		 * @return 绑定是否成功
		 */
		bool Bind(const Address::ptr & addr);
		/**
		 * @brief Connect 连接远程地址
		 *
		 * @param addr 连接的地址
		 * @param timeout 超时时间
		 *
		 * @return 连接是否成功
		 */
		bool Connect(const Address::ptr addr,int timeout = 0);
		/**
		 * @brief reConnect 重连
		 *			需要执行过一个connect
		 * @param timeout 超时时间
		 *
		 * @return 连接是否成功
		 */
		bool reConnect(int timeout = -1);
		/**
		 * @brief Close 关闭socket
		 *
		 * @return 关闭是否成功
		 */
		bool Close() override;

		/**
		 * @brief getLocalAddr 获取本地的绑定地址
		 *
		 * @return 地址对象
		 */
		Address::ptr getLocalAddr();
		/**
		 * @brief getRemoteAddr 获取连接的远程地址
		 *
		 * @return 地址对象
		 */
		Address::ptr getRemoteAddr();
		/**
		 * @brief isConnected 判断socket 是否已连接
		 *
		 * @return 当前的连接状态
		 */
		bool isConnected();

		/**
		 * @brief Recv 接收信息 同recv
		 *
		 * @param buf 接收缓冲区
		 * @param len	缓冲区大小
		 *
		 * @return 接收数据的长度
		 */
		ssize_t Recv(void *buf,size_t len);


		/**
		 * @brief recvAsynCb 添加异步执行套接字recv
		 *									别说开启异步和非阻塞属性
		 * @param cb     回调函数
		 *
		 * @return   返回回调函数是否设置成功
		 */
		bool setAsynReadEvent(AsynFunc cb);

		void sendSignal() override { asynSend();}
		void readSignal() override { asynRead();}

		/**
		 * @brief Recv 发送信息 同recv
		 *
		 * @param buf 发送缓冲区
		 * @param len	缓冲区大小
		 *
		 * @return 发送数据的长度
		 */
		ssize_t Send(void *buf,size_t len);

		ByteArray& getOutputArray() { return m_recvBuf;}
		ByteArray& getInputArray() { return m_sendBuf;}

	private:
		/**
		 * @brief Socket 用于服务端socket accept封装sock
		 *				区别Socket(int domain)
		 * @param sock	socket套接字
		 * @param tmp	socket 是否允许hook
		 */
		Socket(int sock, const Address::ptr& addr);
		void asynRead();
		void asynSend();
		void initSock();

		bool m_connected = false;
		Address::ptr m_bindAddr = nullptr;
		Address::ptr m_destAddr = nullptr;
		//应用层的接受缓冲区
		ByteArray m_recvBuf;
		//应用层的发送缓冲区
		ByteArray	m_sendBuf;
		
		AsynFunc m_readAsynFunc;
	};
}
#endif
