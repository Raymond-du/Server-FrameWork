#ifndef __SOCKETIMPL_H
#define __SOCKETIMPL_H

#include "byteArray.h"
#include <bits/stdint-uintn.h>
#include <memory>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
namespace raymond{

	enum SocketEvent {
		RECV = 1,
		SEND = 2,
		NOKNOW = 0
	};

	class SocketImpl : public std::enable_shared_from_this<SocketImpl> {
	protected:
		int m_sock = -1;
		int m_family = 0;
		uint32_t m_recvTimeOut = -1;
		uint32_t m_sendTimeOut = -1;
		bool m_valid = false;
		bool m_bound = false;
		bool m_nonBlock = false;
		bool m_hookEnable = false;

	private:
		std::string m_name = "";
		//作为和socket绑定的参数
		std::shared_ptr<void> m_arg;
		
	public:
		typedef std::shared_ptr<SocketImpl> ptr;

		SocketImpl() {}
		SocketImpl(int sock) { m_sock = sock;}

		virtual void sendSignal() = 0;
		virtual void readSignal() = 0;

		/**
		 * @brief setSendTimeout 设置发送超时时长
		 *
		 * @param t 超时的时长 ms
		 */
		virtual void setSendTimeout(uint32_t ms);
		/**
		 * @brief getSendTimeout 获取发送的超时时长
		 *
		 * @return 超时的时长 ms
		 */
		virtual uint32_t getSendTimeout();
		/**
		 * @brief setSendTimeout 设置接收超时时长
		 *
		 * @param t 超时的时长 ms
		 */
		virtual void setRecvTimeout(uint32_t t);
		/**
		 * @brief getSendTimeout 获取接收的超时时长
		 *
		 * @return 超时的时长 ms
		 */
		uint32_t getRecvTimeout();

		/**
		 * @brief setHookEnable 设置是否允许hook 
		 *									 设置好后添加到socketmanager里面
		 * @param v
		 *
		 * @return 
		 */
		virtual bool setHookEnable(bool v);

		/**
		 * @brief setNonBlock 设置socket 阻塞或非阻塞类型
		 *
		 * @param b 
		 *
		 * @return 设置是否成功
		 */
		virtual bool setNonBlock(bool v);

		/**
		 * @brief isNonBlock socket是否是非阻塞的
		 *
		 * @return 是否为非阻塞
		 */
		virtual bool isNonBlock() { return m_nonBlock;}

		/**
		 * @brief isHookEnable 返回是否允许hook
		 *
		 * @return 
		 */
		virtual bool isHookEnable() { return m_hookEnable;}

		/**
		 * @brief isBound 防止是否bind端口和ip地址
		 *                
		 * @return 
		 */
		virtual bool isBound() { return m_bound;}

		virtual bool isValid() { return m_valid;}

		std::string toString() const {
			std::stringstream ss;
			ss << "sockFd:" << m_sock << ",family:" << m_family 
				 << ",isValid:" << m_valid << ",isBound:" << m_bound
				 << ",isHookEnable" << m_hookEnable << ",isNonBlock" << m_nonBlock;
			return ss.str();
		}

		template<class t>
		bool setOpt(int level,int option,const t & value){
			return setOpt(level,option,&value,sizeof(t));
		}

		template<class t>
		bool getOpt(int level,int option,t &value){
			socklen_t len = sizeof(t);
			return getOpt(level,option,&value,&len);	
		}

	 	bool setOpt(int level,int option,const void *result,socklen_t len);
		bool getOpt(int level,int option,void *result,socklen_t* len);

		int getSock() const;

		const std::string& getName() { return m_name;}
		void setName(const std::string& name) { m_name = name;}

		void resetArg(std::shared_ptr<void> arg) { m_arg = arg;}
	
		virtual bool Close();
		virtual ~SocketImpl();

	};
}

#endif
