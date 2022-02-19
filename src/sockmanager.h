#ifndef __SOCKMANAGER_H
#define __SOCKMANAGER_H
#include "RWMutex.h"
#include "noncopy.h"
#include "singleton.h"
#include "socketImpl.h"
#include <bits/stdint-uintn.h>
#include <map>
#include <memory>

namespace raymond{
//	class SockContext{
//	public:
//		typedef std::shared_ptr<SockContext> ptr;	
//		
//		enum SockType{
//			RECV,
//			SEND
//		};
//
//		SockContext(int fd);
//
//		/**
//		 * @brief isInit 判断是否被初始化
//		 *
//		 * @return 返回结果
//		 */
//		bool isInit();
//		/**
//		 * @brief isSocket 判断是否是socket
//		 *
//		 * @return 
//		 */
//		bool isSocket();
//		/**
//		 * @brief isClose 判断socket戳否已经关闭
//		 *
//		 * @return 
//		 */
//		bool isClose();
//		/**
//		 * @brief setSysNonblock 设置系统的阻塞情况
//		 *
//		 * @param v
//		 */
//		void setNonblock(bool v);
//		/**
//		 * @brief isSysNonblock 获取系统的组摄状态
//		 *
//		 * @return 
//		 */
//		bool isNonblock();
//		/**
//		 * @brief setTimeOut 设置超时时间
//		 *
//		 * @param type 超时类型是 读还是写
//		 * @param t 超时时间
//		 */
//		void setTimeOut(SockType type,uint32_t t);
//		/**
//		 * @brief getTimeOut 获取对应类型的超时类型
//		 *
//		 * @param type 超时类型 (读或者写)
//		 *
//		 * @return 超时时间
//		 */
//		uint32_t getTimeOut(SockType type);
//
//	private:
//		/**
//		 * @brief 作为sock的上下文的标志
//		 * 最低位到高位
//		 * 1. 是否初始化
//		 * 2. 是否是socket
//		 * 3, 是否非阻塞
//		 * 4. 套接字是否已经关闭
//		 */
//		uint8_t m_flag = 0x00;
//		int m_fd = 0;
//		uint32_t m_recvTimeOut = 0;
//		uint32_t m_sendTimeOut = 0;
//	private:
//		/**
//		 * @brief setInit 设置初始状态
//		 *
//		 * @param v
//		 */
//		void setInit(bool v);
//		/**
//		 * @brief setSock 设置是否是socket
//		 *
//		 * @param v
//		 */
//		void setSock(bool v);
//		/**
//		 * @brief setClose 设置是否已经关闭
//		 *
//		 * @param v
//		 */
//		void setClose(bool v);
//
//	};
	class SockManager{
	public:
		typedef std::shared_ptr<SockManager> ptr;
		SockManager(){}
		bool addSock(SocketImpl::ptr sock);
		SocketImpl::ptr getSock(int fd);
		bool delSock(int fd);
		bool delSock(SocketImpl::ptr sock);
	private:
		RWMutex m_rwMutex;
		std::map<int, SocketImpl::ptr> m_socks;
	};
	//单利模式
	typedef SingleTon<SockManager> sockMgr;	
	
}

#endif

