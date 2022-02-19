#ifndef __ADDRESS_H
#define __ADDRESS_H

#include <bits/stdint-uintn.h>
#include <map>
#include <memory>
#include <ostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <utility>
#include <vector>
namespace raymond{
	/**
	 * @brief Address的基类
	 */
	class Address{
	public:
		typedef std::shared_ptr<Address> ptr;
		virtual ~Address(){}
		/**
		 * @brief Create 根据addr 创建指定的address 对象
		 *	
		 * @param addr sockaddr
		 * @param len	addr的长度
		 *
		 * @return 返回对象的智能指针
		 */
		static Address::ptr createAddr(const sockaddr * addr,socklen_t len);

		/**
		 * @brief getAddrByHost 通过host地址返回address地址
		 *
		 * @param host	host域名/服务器名
		 * @param addrs	结果返回到results
		 * @param service 端口或者服务的名称 (ftp)
		 * @param family	协议族(AF_INET等)
		 * @param socktype	socket类型 (sock_stream)
		 * @param protocol	协议(protocol_TCP)
		 *
		 * @return 返回是否成功
		 */
		static bool getAddrByHost(const char* host,std::vector<Address::ptr>& addrs,
				const char * service = nullptr,int family = AF_INET,
				int socktype = 0,int protocol = 0);
		//获取协议族
		int getFamily();
		//获取地址
		virtual const sockaddr * getAddr() const = 0;
		/**
		 * @brief getAddrLen 获取address的长度
		 */
		virtual socklen_t getAddrLen() const = 0;
		/**
		 * @brief outStream 
		 *
		 * @param os
		 *
		 * @return 
		 */
		virtual std::ostream& outStream(std::ostream & os) const = 0;

		/**
		 * @brief toString address序列化字符串
		 *
		 * @return 
		 */
		std::string toString();
		//方便储存和排序
		bool operator<(const Address & rhs) const;
		bool operator==(const Address & rhs) const;
		bool operator!=(const Address & rhs) const;
	};
	
	class IPAddress : public Address{
	public:
		typedef std::shared_ptr<IPAddress> ptr;

		/**
		 * @brief getIpAddr 更具域名创建一个对应的address
		 *
		 * @param host 域名或者ip地址
		 * @param port	端口
		 * @family 协议族
		 * @socktype sock类型
		 * @protocol 协议
		 *
		 * @return address的智能指针
		 */
		static IPAddress::ptr getIpAddr(const char *host ,uint16_t port = 0,
						int family = AF_UNSPEC,int socktype = 0,int protocol = 0);

		static bool getInterfaceAddr(std::multimap<std::string,
				std::pair<IPAddress::ptr,uint32_t> >& results,int family);

		/**
		 * @brief getIPStr 获取ip字符串序列
		 *
		 * @return 字符串
		 */
		virtual std::string getIPStr()const = 0;
		virtual uint16_t getPort()const = 0;
		virtual void setPort(uint16_t port) = 0;
	};
	
	class IPv4Address : public IPAddress{
	public:
		typedef std::shared_ptr<IPv4Address> ptr;
		IPv4Address(const sockaddr_in & addr);
		/**
		 * @brief IPv4Address 创建一个IPv4Address的对象
		 *
		 * @param addr 字符串地址
		 * @param port	端口
		 */
		IPv4Address(const char * addr = nullptr,uint16_t port = 0);
		/**
		 * @brief getIpv4Addr 获取ipv4的地址对象 
		 *
		 * @param addr 若为空 则表示inaddr_any
		 * @param port	端口号
		 *
		 * @return address的智能直指针
		 */
		static IPv4Address::ptr getIpv4Addr(const char *addr = nullptr,uint16_t port = 0);
		/**
		 * @brief getIpv4AddrStr 获取ipv4的字符串地址
		 *
		 * @param host 域名
		 *
		 * @return 字符串地址
		 */
		static std::string getIpv4AddrStr(const char * host);
	
		const sockaddr * getAddr() const override;
		socklen_t getAddrLen() const override;
		std::ostream& outStream(std::ostream & os) const override;
		
		//获取广播地址
		IPv4Address::ptr getBroadcastAddr(uint32_t prefix_len) ;
		//获取网络地址
		IPv4Address::ptr getNetworkAddr(uint32_t prefix_len) ;
		//获取子网掩码的地址
		IPv4Address::ptr getSubnetMask(uint32_t prefix_len) ;
		std::string getIPStr() const override;
		uint16_t getPort()const override;
		void setPort(uint16_t port) override;
		
	private:
		IPv4Address(uint32_t addr);
		sockaddr_in m_addr;
	};

	class IPv6Address : public IPAddress{
	public:
		typedef std::shared_ptr<IPv6Address> ptr;
		IPv6Address(const sockaddr_in6 & addr);
		IPv6Address(const char * address = nullptr,uint16_t port = 0);
		static IPv6Address::ptr getIpv6Addr(const char *addr = nullptr,uint16_t port = 0);
		static std::string getIpv6AddrStr(const char * host);

		const sockaddr * getAddr() const override;
		socklen_t getAddrLen() const override;
		std::ostream& outStream(std::ostream & os) const override;

		std::string getIPStr() const override;
		uint16_t getPort()const override;
		void setPort(uint16_t port) override;
	private:
		sockaddr_in6 m_addr;
	};

	class UnixAddress : public Address{
	public:
		typedef std::shared_ptr<UnixAddress> ptr;
		UnixAddress(const std::string & path);
		UnixAddress(const sockaddr_un & addr,socklen_t len);

		const sockaddr * getAddr() const override;
		socklen_t getAddrLen() const override;
		std::ostream& outStream(std::ostream & os) const override;

	private:
		sockaddr_un m_addr;	
		socklen_t m_len = 0;
	};

	class UnKownAddress : public Address{
	public:
		typedef std::shared_ptr<UnKownAddress> ptr;
		UnKownAddress(int family);
		UnKownAddress(const sockaddr & addr);

		const sockaddr * getAddr() const override;
		socklen_t getAddrLen() const override;
		std::ostream& outStream(std::ostream & os) const override;
	private:
		sockaddr m_addr;
	};
}
#endif
