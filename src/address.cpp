#include "address.h"
#include <exception>
#include <memory>
#include <netdb.h>
#include "hook.h"
#include <arpa/inet.h>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "Logger.h"
#include <ifaddrs.h>
#include <utility>
#include <vector>

namespace raymond{
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	template<class T>
		/**
		 * @brief getMaskPrefix 根据value获取prefix_len
		 *
		 * @param value 
		 *
		 * @return prefix_len
		 */
	static uint32_t getMaskPrefix(T value){
		uint32_t len = 0;
		for(;value;value = value >> 1){
			++len;
		}
		return len;
	}
	
	template<class T>
		/**
		 * @brief CreateMask 返回对应位数的掩码
		 *
		 * @param bits 掩码的位数
		 */
	static T CreateMask(uint32_t bits){
		//sizeof(T) * 8 返回的位数
		return ~(T)1 << (sizeof(T) * 8 - bits - 1);
	}

	int Address::getFamily(){
		return getAddr()->sa_family;
	}
	
	std::string Address::toString(){
		std::stringstream ss;
		outStream(ss);
		return ss.str();
	}

	Address::ptr Address::createAddr(const sockaddr * addr,socklen_t len){
		if(addr == nullptr){
			return nullptr;
		}

		Address::ptr rt = nullptr;
		switch(addr->sa_family){
		case AF_INET:
			rt.reset(new IPv4Address(*(sockaddr_in *)addr));
			break;
		case AF_INET6:
			rt.reset(new IPv6Address(*(sockaddr_in6*)addr));
			break;
		case AF_UNIX:
			rt.reset(new UnixAddress(*(sockaddr_un*)addr,len));
			break;
		default:
			rt.reset(new UnKownAddress(*addr));
			break;
		}
		return rt;
	}

	bool Address::getAddrByHost(const char *host,std::vector<Address::ptr> &addrs,
			const char * service,int family ,int socktype,int protocol){
		addrinfo hints,*results;
		memset(&hints,'\0',sizeof(addrinfo));
		hints.ai_family = family;
		hints.ai_socktype = socktype;
		hints.ai_protocol = protocol;
		hints.ai_flags = AI_ALL | AI_PASSIVE;

		int rt = getaddrinfo(host,service,&hints,&results);
		if(rt){
			RAYMOND_LOG_FMT_ERROR(g_logger,"getaddrinfo has error : %s",gai_strerror(rt));
			return false;
		}
		try {
			for(auto result = results;result;result = result->ai_next){
				addrs.push_back(Address::createAddr(result->ai_addr,result->ai_addrlen));
			}
			freeaddrinfo(results);
			return true;
		}catch (...){
			RAYMOND_LOG_FMT_ERROR(g_logger,"getaddrinfo has error");
			freeaddrinfo(results);
			return false;
		}
	}

	bool Address::operator<(const Address & rhs) const{
		socklen_t minlen = std::min(getAddrLen(),rhs.getAddrLen());
		int rt = memcmp(getAddr(),rhs.getAddr(),minlen);
		if(rt < 0){
			return true;
		}else if(getAddrLen() < rhs.getAddrLen()){
			return true;
		}
		return false;
	}

	bool Address::operator==(const Address & rhs) const{
		return getAddrLen() == rhs.getAddrLen() &&
			memcmp(getAddr(),rhs.getAddr(),getAddrLen()) == 0;
	}
	
	bool Address::operator!=(const Address & rhs) const{
		return !(*this == rhs);
	}
	
	IPAddress::ptr IPAddress::getIpAddr(const char *host,uint16_t port,
							int family,int socktype,int protocol){
		addrinfo hints,*results;
		memset(&hints,'\0',sizeof(addrinfo));
		hints.ai_family = family;
		hints.ai_socktype = socktype;
		hints.ai_protocol = protocol;
		hints.ai_flags = AI_ALL | AI_PASSIVE;

		if(host == nullptr){
			RAYMOND_LOG_FMT_ERROR(g_logger, "host is nullptr");
			return nullptr;
		}

		int rt = getaddrinfo(host,nullptr,&hints,&results);
		if(rt){
			RAYMOND_LOG_FMT_ERROR(g_logger,"getaddrinfo has error : %s",gai_strerror(rt));
			return nullptr;
		}
		try {
			auto result = std::dynamic_pointer_cast<IPAddress>(
					Address::createAddr(results->ai_addr,results->ai_addrlen));
			if(result){
				result->setPort(port);
			}
			freeaddrinfo(results);
			return result;
		}catch (...){
			RAYMOND_LOG_FMT_ERROR(g_logger,"dynamic_pointer_cast has error");
			freeaddrinfo(results);
			return nullptr;
		}
	}

	bool IPAddress::getInterfaceAddr(std::multimap<std::string,
			std::pair<IPAddress::ptr,uint32_t> >& addrs,int family){
		ifaddrs *results;
		if(-1 == getifaddrs(&results)){
			RAYMOND_LOG_FMT_ERROR(g_logger,"getInterfaceAddr has error : %s",
							std::strerror(errno));
			return false;
		}
		try{	
			for(auto result = results;result;result = result->ifa_next){
				IPAddress::ptr ipAddr = nullptr;
				uint32_t prefix_len = 0;
				if(family != AF_UNSPEC && family != result->ifa_addr->sa_family){
					continue;
				}
				switch(result->ifa_addr->sa_family){
				case AF_INET:	
					{
						ipAddr = std::dynamic_pointer_cast<IPAddress>(
								Address::createAddr(result->ifa_addr,sizeof(sockaddr_in)));
						auto netMask = (sockaddr_in*)result->ifa_netmask;				
						prefix_len = getMaskPrefix(netMask->sin_addr.s_addr);
					}
					break;
				case AF_INET6:
					{
						ipAddr = std::dynamic_pointer_cast<IPAddress>(
								Address::createAddr(result->ifa_addr,sizeof(sockaddr_in6)));
						auto netMask = (sockaddr_in6*)result->ifa_netmask;
						for(int i = 0;i < 16 ;i++){
							prefix_len += getMaskPrefix(netMask->sin6_addr.s6_addr[i]);
						}
					}
					break;
				default:
					break;
				}
				if(ipAddr){
					addrs.insert(std::make_pair(std::string(result->ifa_name),
								std::make_pair(ipAddr,prefix_len)));
				}
			}
		}catch (std::exception e){
			freeifaddrs(results);
			RAYMOND_LOG_FMT_ERROR(g_logger,"getInterfaceAddr errno %s",e.what());
			return false;
		}
		
		freeifaddrs(results);
		return !addrs.empty();
	}

	IPv4Address::IPv4Address(const sockaddr_in & addr){
		m_addr = addr;
	}
	
	IPv4Address::IPv4Address(const char * addr,uint16_t port){
		memset(&m_addr,'\0',sizeof(sockaddr_in));
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		if(addr == nullptr){
			m_addr.sin_addr.s_addr = INADDR_ANY;
		}else{
			int rt = inet_pton(AF_INET,addr,&m_addr.sin_addr);
			if(rt == 0){
				RAYMOND_LOG_FMT_ERROR(g_logger,"address is invalid");
			}else if(rt == -1){
				RAYMOND_LOG_FMT_ERROR(g_logger,"address family is invalid");
			}
		}
	}
	
	IPv4Address::IPv4Address(uint32_t addr){
		memset(&m_addr,'\0',sizeof(sockaddr_in));
		m_addr.sin_family = AF_INET;
		m_addr.sin_addr.s_addr = htonl(addr);
	}
	
	IPv4Address::ptr IPv4Address::getIpv4Addr(const char * host,uint16_t port){
		auto addr = IPAddress::getIpAddr(host,port,AF_INET);
		if(addr == nullptr){
			return IPv4Address::ptr(new IPv4Address(nullptr,port));
		}	
		return std::dynamic_pointer_cast<IPv4Address>(addr);
	}
	
	std::string IPv4Address::getIpv4AddrStr(const char * host){
		auto addr = IPAddress::getIpAddr(host,0,AF_INET);
		return addr->getIPStr();
	}

	const sockaddr * IPv4Address::getAddr() const {
		return (sockaddr *)&m_addr;
	}

	socklen_t IPv4Address::getAddrLen() const {
		return sizeof(sockaddr_in);
	}

	std::ostream& IPv4Address::outStream(std::ostream & os) const {
		os<<getIPStr()<<":"<<ntohs(m_addr.sin_port);
		return os;
	}

	IPv4Address::ptr IPv4Address::getBroadcastAddr(uint32_t prefix_len) {
		if(prefix_len > 32){
			return nullptr;
		}
		
		uint32_t mask = CreateMask<uint32_t>(prefix_len);
		uint32_t b_addr = htonl(m_addr.sin_addr.s_addr) | ~mask;
		return IPv4Address::ptr(new IPv4Address(b_addr));
	}

	IPv4Address::ptr IPv4Address::getNetworkAddr(uint32_t prefix_len) {
		if(prefix_len > 32){
			return nullptr;
		}
		uint32_t mask = CreateMask<uint32_t> (prefix_len);
		uint32_t n_addr = htonl(m_addr.sin_addr.s_addr) & mask;
		return IPv4Address::ptr(new IPv4Address(n_addr));
	}
	
	IPv4Address::ptr IPv4Address::getSubnetMask(uint32_t prefix_len) {
		return IPv4Address::ptr(new IPv4Address(CreateMask<uint32_t>(prefix_len)));
	}

	std::string IPv4Address::getIPStr() const{
		int32_t hostOrder = ntohl(m_addr.sin_addr.s_addr);
		std::stringstream ss;
		ss<<((hostOrder >> 24) & 0xff)<<"."
			<<((hostOrder >> 16) & 0xff) << "."
			<<((hostOrder >> 8) & 0xff) << "."
			<<(hostOrder & 0xff);
		return ss.str();	
	}

	uint16_t IPv4Address::getPort()const {
		return ntohs(m_addr.sin_port);
	}

	void IPv4Address::setPort(uint16_t port) {
		m_addr.sin_port = htons(port);
	}
	
	IPv6Address::IPv6Address(const sockaddr_in6 & addr){
		m_addr = addr;
	}
	
	IPv6Address::IPv6Address(const char *address ,uint16_t port){
		memset(&m_addr,0,sizeof(sockaddr_in6));
		m_addr.sin6_family = AF_INET6;
		m_addr.sin6_port = htons(port);
		if(address == nullptr){
			m_addr.sin6_addr = in6addr_any;
		}else{
			int rt = inet_pton(AF_INET6,address,m_addr.sin6_addr.s6_addr);
			if(rt == 0){
				RAYMOND_LOG_FMT_ERROR(g_logger,"address is invalid");
			}else if(rt == -1){
				RAYMOND_LOG_FMT_ERROR(g_logger,"address family is invalid");
			}	
		}
	}

	IPv6Address::ptr IPv6Address::getIpv6Addr(const char * host,uint16_t port){
		auto addr = IPAddress::getIpAddr(host,port,AF_INET6);
		if(addr == nullptr){
			return IPv6Address::ptr(new IPv6Address(nullptr,port));
		}
		return std::dynamic_pointer_cast<IPv6Address>(addr);
	}	

	std::string IPv6Address::getIpv6AddrStr(const char * host){
		auto addr = IPAddress::getIpAddr(host,0,AF_INET6);
		return addr->getIPStr();
	}

	const sockaddr * IPv6Address::getAddr() const {
		return (sockaddr *)&m_addr;
	}
	
	socklen_t IPv6Address::getAddrLen() const {
		return sizeof(sockaddr_in6);
	}
	
	std::ostream& IPv6Address::outStream(std::ostream & os) const {
		os<<"["<<getIPStr()<<"]:"<<ntohs(m_addr.sin6_port);
		return os;
	}

	std::string IPv6Address::getIPStr() const{
		char addr[INET6_ADDRSTRLEN] {0};
		if(nullptr == inet_ntop(AF_INET6,m_addr.sin6_addr.s6_addr,addr,INET6_ADDRSTRLEN)){
			RAYMOND_LOG_FMT_ERROR(g_logger,"inet_ntop has error %s",std::strerror(errno));
			return "";
		}
		return std::string(addr);
	}

	uint16_t IPv6Address::getPort()const {
		return m_addr.sin6_port;
	}
	
	void IPv6Address::setPort(uint16_t port) {
		m_addr.sin6_port = htons(port);
	}
	
	UnixAddress::UnixAddress(const std::string & path){
		memset(&m_addr,0,sizeof(sockaddr_un));
		m_addr.sun_family = AF_UNIX;
		m_len = path.size() + 1;

		//表示unix套接字的抽象路径名 (不会创建真实的文件)
		if(!path.empty() && path[0] == '\0'){
			--m_len;
		}

		if(m_len > sizeof(sockaddr_un::sun_path)){
			throw std::logic_error("unix path too long");
		}
		memcpy(m_addr.sun_path,path.c_str(),m_len);
		m_len += offsetof(sockaddr_un,sun_path);
	}

	UnixAddress::UnixAddress(const sockaddr_un & addr,socklen_t len){
		m_addr = addr;
		m_len = len;
	}

	const sockaddr * UnixAddress::getAddr() const {
		return (sockaddr *)&m_addr;
	}

	socklen_t UnixAddress::getAddrLen() const {
		return m_len;
	}
	
	std::ostream& UnixAddress::outStream(std::ostream & os) const {
		//表示抽象的文件路径
		if(m_len > offsetof(sockaddr_un,sun_path) &&
				m_addr.sun_path[0] == '\0'){
			return os<<"\\0"<<std::string(m_addr.sun_path + 1,
					m_len - offsetof(sockaddr_un,sun_path) - 1);
		}
		return os<<m_addr.sun_path;
	}

	UnKownAddress::UnKownAddress(int family){
		memset(&m_addr,'\0',sizeof(sockaddr));
		m_addr.sa_family = family;
	}

	UnKownAddress::UnKownAddress(const sockaddr & addr){
		m_addr = addr;
	}

	const sockaddr * UnKownAddress::getAddr() const {
		return &m_addr;
	}
	
	socklen_t UnKownAddress::getAddrLen() const {
		return sizeof(sockaddr);
	}
	
	std::ostream& UnKownAddress::outStream(std::ostream & os) const {
		return os<<"[UnKownAddress family = ]"<<m_addr.sa_family;
	}

}
