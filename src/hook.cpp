#include "hook.h"
#include "Logger.h"
#include "config.h"
#include "fiber.h"
#include "iomanager.h"
#include "macro.h"
#include "schedule.h"
#include "socketImpl.h"
#include "timer1.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <bits/types/struct_iovec.h>
#include <bits/types/struct_timespec.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <ctime>
#include <dlfcn.h>
#include <fcntl.h>
#include <memory>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <yaml-cpp/exceptions.h>
#include "sockmanager.h"



namespace raymond{
	static raymond::Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");
	//设置连接超时的配置 和 设置读和写的超时配置
	static auto g_connect_timeout = raymond::Config::lookUp("socket.connect.timeout",5000,"socket connect timeout");
	static auto g_recv_timeout = raymond::Config::lookUp("socket.recv.timeout",5000,"socket recv timeout");
	static auto g_sock_nonblock = raymond::Config::lookUp("socket.sock.isnonblock",true,"socket recv send ... flag is nonblock");
	static auto g_send_timeout = raymond::Config::lookUp("socket.send.timeout",5000,"socket send timeout");

	static thread_local bool t_hook_enable = false;

	bool is_hook_enable() {
		return t_hook_enable;
	}

	void set_hook_enable(bool flag) {
		t_hook_enable = flag;
	}

#define HOOK_FUN(xx) \
	xx(sleep)		\
	xx(usleep)		\
	xx(nanosleep)   \
	xx(socket)		\
	xx(connect)		\
	xx(accept)		\
	xx(read)		\
	xx(readv)		\
	xx(recv)		\
	xx(recvfrom)	\
	xx(recvmsg)		\
	xx(write)		\
	xx(writev)		\
	xx(send)		\
	xx(sendto)		\
	xx(sendmsg)		\
	xx(close)		\
	xx(fcntl)		\
	xx(ioctl)		\
	xx(getsockopt)	\
	xx(setsockopt)	

	void hook_init(){
		static bool is_inited = false;
		if(is_inited){
			return;
		}
		//将原始的函数保存到name+_f的函数中
#define xx(name) name ## _f = (name ## _func) dlsym(RTLD_NEXT,#name);
		HOOK_FUN(xx);
#undef xx
	}

	struct _InitHook{
		_InitHook(){
			hook_init();
		}
	};
	
	static _InitHook s_hook_init;
}

//用来定义一个条件变量
struct TCondition{
	int canceled = 0;
};
	
//template<typename F,typename... Args>
//static ssize_t do_io(int fd,F&& func,const std::string & func_name,
//			raymond::IOManager::Event event,raymond::SockContext::SockType timeout_type,Args... args){
//	
//	if(!raymond::is_hook_enable()){
//		return func(fd,std::forward<Args>(args)...);
//	}
//
//	auto sock = raymond::sockMgr::getInstance()->getSock(fd);
//	if(!sockCtx || !sockCtx->isNonblock()){
//		return func(fd,std::forward<Args>(args)...);
//	}
//
//	if(sockCtx->isClose()){
//		errno = EBADF;
//		return -1;
//	}
//	
//	//执行func的操作 如果是终端信号 则继续执行
//	ssize_t n = 0;
//	//获取超时的时间
//	uint32_t timeOut = sockCtx->getTimeOut(timeout_type);
//	do{
//		n = func(fd,std::forward<Args>(args)...);
//		if(n == -1 && errno == EINTR){
//			continue;
//		}else
//		//socket处于非阻塞的情况下 没有获得信息 执行异步的操作
//		if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
//			auto ioManage = raymond::IOManager::getIOManager();
//			raymond::Timer::ptr timer = nullptr;
//			auto fiber = raymond::Fiber::getCurFiber();
//			
//			std::shared_ptr<TCondition> sInfo(new TCondition);
//			std::weak_ptr<TCondition> wInfo(sInfo);
//			//判断是否设置超时时间
//			if(timeOut != (uint32_t)-1){
//				//如果计时结束 winfo 不存在了 说明没有超时,已经被执行结束了
//				//如果winfo 存在 则取消fd的监听
//				timer = ioManage->addTimer(timeOut,[ioManage,fd,event,wInfo,fiber](){
//					RAYMOND_LOG_FMT_WARN(raymond::g_logger,"file description fd %d has timeOut", fd);
//
//					auto sInfo = wInfo.lock();
//					if(!sInfo){
//						return;
//					}
//					ioManage->schedule(fiber);
//					sInfo->canceled = ETIMEDOUT;
//					ioManage->delEvent(fd,event);
//				},0);
//			}
//			//监听fd 在超时时间内等待消息的到来 如果到来执行接下来的函数
//			bool rt = ioManage->addEvent(fd,event);
//			if(rt){
//				fiber->swapToHold();
//				//如果canceled被设置为其他值表示超时
//				if(sInfo->canceled){
//					errno = sInfo->canceled;
//					sInfo.reset();
//					return -1;
//				}
//				if(timer){
//					ioManage->delTimer(timer);
//				}
//			}else{
//				RAYMOND_LOG_FMT_ERROR(raymond::g_logger,"addEvent %d %s has errer"
//						,fd,raymond::IOManager::toString(event).c_str());
//				if(timer){
//					ioManage->delTimer(timer);
//				}
//				return -1;
//			}
//		}else{
//			//获取到信息 或者是其他的错误
//			break;
//		}
//	}while(1);
//
//	return n;
//}

template<typename F,typename... Args>
static ssize_t do_io(raymond::SocketImpl::ptr sock,F&& func,const std::string & func_name,
										uint32_t timeout, raymond::IOManager::Event event,Args... args){
	
	if(!raymond::is_hook_enable()){
		return func(sock->getSock(), std::forward<Args>(args)...);
	}

	if(!sock || !sock->isNonBlock()){
		return func(sock->getSock(), std::forward<Args>(args)...);
	}

	if(!sock->isValid()){
		errno = EBADF;
		return -1;
	}
	
	//执行func的操作 如果是终端信号 则继续执行
	ssize_t n = 0;
	do{
		n = func(sock->getSock(), std::forward<Args>(args)...);
		if(n == -1 && errno == EINTR){
			continue;
		}else
		//socket处于非阻塞的情况下 没有获得信息 执行异步的操作
		if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
			auto ioManage = raymond::IOManager::getIOManager();
			raymond::Timer::ptr timer = nullptr;
			auto fiber = raymond::Fiber::getCurFiber();
			
			std::shared_ptr<TCondition> sInfo(new TCondition);
			std::weak_ptr<TCondition> wInfo(sInfo);
			//判断是否设置超时时间
			if(timeout != (uint32_t)-1){
				//如果计时结束 winfo 不存在了 说明没有超时,已经被执行结束了
				//如果winfo 存在 则取消fd的监听
				timer = ioManage->addTimer(timeout,[ioManage, sock,event,wInfo,fiber](){
					RAYMOND_LOG_FMT_WARN(raymond::g_logger,"socket :%s has timeOut", sock->toString().c_str());

					auto sInfo = wInfo.lock();
					if(!sInfo){
						return;
					}
					ioManage->schedule(fiber);
					sInfo->canceled = ETIMEDOUT;
					ioManage->delEvent(sock,event);
				},0);
			}
			//监听fd 在超时时间内等待消息的到来 如果到来执行接下来的函数
			bool rt = ioManage->addEvent(sock,event);
			if(rt){
				fiber->swapToHold();
				//如果canceled被设置为其他值表示超时
				if(sInfo->canceled){
					errno = sInfo->canceled;
					sInfo.reset();
					return -1;
				}
				if(timer){
					ioManage->delTimer(timer);
				}
			}else{
				RAYMOND_LOG_FMT_ERROR(raymond::g_logger,"socket: %s,addevent %s has errer"
						,sock->toString().c_str(),raymond::IOManager::toString(event).c_str());
				if(timer){
					ioManage->delTimer(timer);
				}
				return -1;
			}
		}else{
			//获取到信息 或者是其他的错误
			break;
		}
	}while(1);

	return n;
}

extern "C"{
#define xx(name) name ## _func name ## _f  = nullptr ;
	HOOK_FUN(xx);
#undef xx

	unsigned int sleep(unsigned int seconds){
		if(!raymond::is_hook_enable()){
			return sleep_f(seconds);
		}

		auto fiber = raymond::Fiber::getCurFiber();
		raymond::IOManager* ioManager = raymond::IOManager::getIOManager();
		//使用定时器
		ioManager->addTimer(seconds * 1000,[ioManager,fiber](){
			ioManager->schedule(fiber);
		},0);

		fiber->swapToHold();
		RAYMOND_LOG_FMT_DEBUG(raymond::g_logger,"continue fiber");
		return 0;
	}

	int usleep(useconds_t usec){
		if(!raymond::is_hook_enable()){
			return usleep_f(usec);
		}
		auto fiber = raymond::Fiber::getCurFiber();
		auto ioManage = raymond::IOManager::getIOManager();

		ioManage->addTimer(usec / 1000,[ioManage,fiber](){
			ioManage->schedule(fiber);
		},0);
		fiber->swapToHold();
		return 0;
	}

	int nanosleep(const struct timespec *req, struct timespec *rem){
		if(!raymond::is_hook_enable()){
			return nanosleep_f(req,rem);
		}
		auto fiber = raymond::Fiber::getCurFiber();
		raymond::IOManager* ioManager = raymond::IOManager::getIOManager();

		timespec execTime = *req;
		timespec tmp {0};
		ioManager->addTimer(execTime,[ioManager,fiber](){
			ioManager->schedule(fiber);
		},tmp);
		fiber->swapToHold();
		return 0;	
	}

	//进行使用sockmanager进行管理
	int socket(int domain, int type, int protocol){
		int rt = socket_f(domain,type,protocol);
		//if(raymond::is_hook_enable()){
		//	if(rt != -1){
		//		auto sockCtx = raymond::sockMgr::getInstance()->get(rt);
		//		if (sockCtx) {
		//			RAYMOND_LOG_FMT_DEBUG(raymond::g_logger,"socket %d in socketManager",rt);
		//			if (sockCtx->getTimeOut(raymond::SockContext::RECV) == 0) {
		//				sockCtx->setTimeOut(raymond::SockContext::RECV,
		//									raymond::g_recv_timeout->getValue());
		//			}
		//			if (sockCtx->getTimeOut(raymond::SockContext::SEND) == 0) {
		//				sockCtx->setTimeOut(raymond::SockContext::SEND,
		//									raymond::g_send_timeout->getValue());
		//			}
		//		}
		//		
		//	}
		//}
		return rt;
	}	

	//主要进行设置超时连接
	int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen){
		return connect_with_timeout(sockfd,addr,addrlen,0);
	}

	int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return accept_f(sockfd, addr, addrlen);
		}
		return do_io(sock, accept_f,"accept",sock->getRecvTimeout(), raymond::IOManager::READ,addr,addrlen);
	}

	ssize_t read(int fd,void *buf,size_t count){
		auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		if (sock == nullptr) {
			return read_f(fd, buf, count);
		}
		return do_io(sock,read_f,"read", sock->getRecvTimeout(),raymond::IOManager::READ,buf,count);
	}

	ssize_t readv(int fd,const struct iovec *iov,int iovcnt){
		auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		if (sock == nullptr) {
			return readv_f(fd, iov, iovcnt);
		}
		return do_io(sock ,readv_f,"readv", sock->getRecvTimeout(),raymond::IOManager::READ,iov,iovcnt);
	}

	ssize_t recv(int sockfd,void *buf,size_t len,int flag){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return recv_f(sockfd, buf, len, flag);
		}
		return do_io(sock,recv_f,"recv", sock->getRecvTimeout(),raymond::IOManager::READ,buf,len,flag);
	}

	ssize_t recvfrom (int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return recvfrom_f(sockfd, buf, len, flags, src_addr, addrlen);
		}
		return do_io(sock,recvfrom_f,"recvfrom", sock->getRecvTimeout(), raymond::IOManager::READ,buf,len,flags,src_addr,addrlen);
	}

	ssize_t recvmsg (int sockfd, struct msghdr *msg, int flags){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return recvmsg_f(sockfd, msg, flags);
		}
		return do_io(sock,recvmsg_f,"recvmsg", sock->getRecvTimeout(),raymond::IOManager::READ,msg,flags);
	}

	ssize_t write (int fd, const void *buf, size_t count){
		auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		if (sock == nullptr) {
			return write_f(fd, buf, count);
		}
		return do_io(sock,write_f,"write", sock->getSendTimeout(),raymond::IOManager::WRITE,buf,count);
	}
	ssize_t writev (int fd, const struct iovec *iov, int iovcnt){
		auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		if (sock == nullptr) {
			return writev_f(fd, iov, iovcnt);
		}
		return do_io(sock,write_f,"write",sock->getSendTimeout(), raymond::IOManager::WRITE,iov,iovcnt);
	}

	ssize_t send (int sockfd, const void *buf, size_t len, int flags){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return send_f(sockfd, buf, len, flags);
		}
		return do_io(sock,send_f,"send",sock->getSendTimeout(), raymond::IOManager::WRITE,buf, len,flags);
	}

	ssize_t sendto (int sockfd, const void *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return sendto_f(sockfd, buf, len, flags, dest_addr, addrlen);
		}
		return do_io(sock,sendto_f,"sendto",sock->getSendTimeout(), raymond::IOManager::WRITE,buf,len,flags,dest_addr,addrlen);
	}

	ssize_t sendmsg (int sockfd, const struct msghdr *msg, int flags){
		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		if (sock == nullptr) {
			return sendmsg_f(sockfd, msg, flags); 
		}
		return do_io(sock,sendmsg_f,"sendmsg",sock->getSendTimeout(), raymond::IOManager::WRITE,msg,flags);
	}


	int close (int fd){
		//即使不是hook_endable的线程 也应该检测一下sockcontex
		auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		if(sock && sock->isNonBlock()){
			auto ioManage = raymond::IOManager::getIOManager();
			ioManage->delFdEvent(sock);
			raymond::sockMgr::getInstance()->delSock(sock);
		}
		return close_f(fd);
	}
	
	//设置阻塞 并更新到sockcontext
	int fcntl (int fd, int cmd, ... /* arg */ ){
		va_list va;
		va_start(va,cmd);
		void* arg = va_arg(va, void*);
		va_end(va);
		return fcntl_f(fd, cmd, arg);
		//switch(cmd){
		//	case F_SETFL:
		//		{
		//			//设置文件描述符flags (只读 只写 阻塞 非阻塞等)
		//			int arg = va_arg(va,int);
		//			va_end(va);
		//			if (arg & O_NONBLOCK) {
		//				auto sock = raymond::sockMgr::getInstance()->getSock(fd);
		//				if(sock && !sock->isValid()) {
		//					sockCtx->setNonblock(true);
		//				}
		//			}
		//			return fcntl_f(fd,cmd,arg);
		//		}
		//		break;
		//	case F_DUPFD:
		//	case F_DUPFD_CLOEXEC:
		//	case F_SETFD:
		//	case F_SETOWN:
		//	case F_SETSIG:
		//	case F_SETLEASE:
		//	case F_NOTIFY:
		//#ifdef F_SETPIPE_SZ
		//	case F_SETPIPE_SZ:
		//#endif
		//		{
		//			int arg = va_arg(va,int);
		//			va_end(va);
		//			return fcntl_f(fd,cmd,arg);
		//		}
		//		break;
		//	case F_GETFD:
		//	case F_GETOWN:
		//	case F_GETSIG:
		//	case F_GETLEASE:
		//#ifdef F_GETPIPE_SZ
		//	case F_GETPIPE_SZ:
		//#endif
		//	case F_GETFL:
		//		{
		//			va_end(va);
		//			return fcntl_f(fd,cmd);
		//		}
		//		break;
		//	case F_SETLK:
		//	case F_SETLKW:
		//	case F_GETLK:
		//		{
		//			flock *arg = va_arg(va,flock *);
		//			va_end(va);
		//			return fcntl_f(fd,cmd,arg);
		//		}
		//		break;
		//	case F_GETOWN_EX:
		//	case F_SETOWN_EX:
		//		{
		//			f_owner_ex* arg = va_arg(va,f_owner_ex * );
		//			va_end(va);
		//			return fcntl_f(fd,cmd,arg);
		//		}
		//		break;
		//	default:
		//		va_end(va);
		//		return fcntl_f(fd,cmd);
		//		break;

		//};
	}

	int ioctl (int fd, unsigned long request, ...){
		va_list va;
		va_start(va,request);
		void *arg = va_arg(va,void *);
		va_end(va);
		return ioctl_f(fd, request, arg);

		//if(FIONBIO == request){
		//	bool nonblock = !!(int *)arg;
		//	if (nonblock) {
		//		auto sockCtx = raymond::sockMgr::getInstance()->get(fd);
		//		if(sockCtx && !sockCtx->isClose() && sockCtx->isSocket()){
		//			sockCtx->setNonblock(true);
		//		}
		//	}
		//}
		//return ioctl_f(fd,request,arg);
	}

	int getsockopt (int sockfd, int level, int optname,void *optval, socklen_t *optlen){
		return getsockopt_f(sockfd,level,optname,optval,optlen);
	}

	int setsockopt (int sockfd, int level, int optname,const void *optval, socklen_t optlen){
		return setsockopt_f(sockfd, level, optname, optval, optlen);
		//if(!raymond::is_hook_enable()){
		//	return setsockopt_f(sockfd,level,optname,optval,optlen);
		//}
		//if(level == SOL_SOCKET){
		//	if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO){
		//		auto sockCtx = raymond::sockMgr::getInstance()->get(sockfd);
		//		if(sockCtx){
		//			const timeval * v = (const timeval *)optval;
		//			raymond::SockContext::SockType type = (optname == SO_RCVTIMEO ? 
		//				raymond::SockContext::RECV : raymond::SockContext::SEND);
		//			sockCtx->setTimeOut(type,v->tv_sec * 1000 + v->tv_usec / 1000);
		//		}
		//	}
		//}
		//return setsockopt_f(sockfd,level,optname,optval,optlen);
	}

	int connect_with_timeout(int sockfd, const sockaddr* addr, socklen_t addrlen, uint32_t timeout_ms){
		int rt = connect_f(sockfd,addr,addrlen);
		if(!raymond::is_hook_enable()){
			return rt;
		}

		auto sock = raymond::sockMgr::getInstance()->getSock(sockfd);
		//如果不在socketmanager中 不是socket或者是阻塞就直接返回
		if (!sock || !sock->isValid() || !sock->isNonBlock()){
			return rt;
		}
		
		if(rt == 0){
			//表示连接成功
			return 0;
		}else if(rt == -1 && errno != EINPROGRESS){
			//EINPROGRESS 表示fd是非阻塞的且连接超时
			return rt;
		}
		//连接事件监听
		auto ioManage = raymond::IOManager::getIOManager();
		raymond::Timer::ptr timer = nullptr;
		std::shared_ptr<TCondition> sInfo(new TCondition);
		std::weak_ptr<TCondition> wInfo(sInfo);
		auto fiber = raymond::Fiber::getCurFiber();
		
		uint32_t timeOut = (timeout_ms == 0 ? raymond::g_connect_timeout->getValue() :
												timeout_ms);
		if(timeOut != (uint32_t)-1){
			//添加定时器 如果超时则删除对sockfd的监听
			timer = ioManage->addTimer(timeOut,[wInfo,ioManage,sock ,fiber](){
				auto sInfo = wInfo.lock();
				if(!sInfo){
					return ;
				}
				sInfo->canceled = ETIMEDOUT;
				ioManage->schedule(fiber);
				ioManage->delEvent(sock,raymond::IOManager::WRITE);
			},0);
		}
		
		rt = ioManage->addEvent(sock,raymond::IOManager::WRITE);
		if(rt){
			raymond::Fiber::yeildToHold();
			if(timer){
				ioManage->delTimer(timer);
			}
			if(sInfo->canceled){
				errno = sInfo->canceled;
				return -1;
			}
		}else{
			if(timer){
				ioManage->delTimer(timer);
			}
			RAYMOND_LOG_FMT_ERROR(raymond::g_logger,"add fd:%d connect event has errer",sockfd);
		}
		//判断sockfd是否正常
		int error = 0;
		socklen_t len = sizeof(int);
		if(-1 == getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len)){
			return -1;
		}
		if(!error){
			return 0;
		}else{
			errno = error;
			return -1;
		}
	}
	
}
