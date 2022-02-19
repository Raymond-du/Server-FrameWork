
#ifndef __HOOK_H
#define __HOOK_H

#include <bits/stdint-uintn.h>
#include <bits/types/struct_timespec.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
namespace raymond{
	/**
	 * @brief is_hook_enable 判断当前线程是否允许被hook
	 *
	 * @return bool
	 */
	bool is_hook_enable();

	/**
	 * @brief set_hook_enable 设置当前线程允许被hook
	 *
	 * @param flag bool
	 */
	void set_hook_enable(bool flag);

}
//使用c风格
extern "C"{
	typedef unsigned int (*sleep_func)(unsigned int seconds);
	//extern 表示定义在其他的文件中  sleep_f 保存这原本的函数
	extern sleep_func sleep_f;

	typedef int (*nanosleep_func)(const timespec*req,timespec *rem);
	extern nanosleep_func nanosleep_f;

	typedef int (*usleep_func)(useconds_t usec);
	extern usleep_func usleep_f;

	//socket相关
	typedef int (*socket_func)(int domain, int type, int protocol);
	extern socket_func socket_f;

	typedef int (*connect_func)(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
	extern connect_func connect_f;

	typedef int (*accept_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	extern accept_func accept_f;
	
	//读相关的
	typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
	extern read_func read_f;

	//循环读入到iovec的数组中
	typedef ssize_t (*readv_func)(int fd, const struct iovec *iov, int iovcnt);
	extern readv_func readv_f;

	typedef ssize_t (*recv_func)(int sockfd, void *buf, size_t len, int flags);
	extern recv_func recv_f;

	typedef ssize_t (*recvfrom_func)(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
	extern recvfrom_func recvfrom_f;

	typedef ssize_t (*recvmsg_func)(int sockfd, struct msghdr *msg, int flags);
	extern recvmsg_func recvmsg_f;
	
	//写相关
	typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
	extern write_func write_f;
	//循环将iov中的数据写到fd中
	typedef ssize_t (*writev_func)(int fd, const struct iovec *iov, int iovcnt);
	extern writev_func writev_f;

	typedef ssize_t (*send_func)(int sockfd, const void *buf, size_t len, int flags);
	extern send_func send_f;

	typedef ssize_t (*sendto_func)(int sockfd, const void *buf, size_t len, int flags,const struct sockaddr *dest_addr, socklen_t addrlen);
	extern sendto_func sendto_f;

	typedef ssize_t (*sendmsg_func)(int sockfd, const struct msghdr *msg, int flags);
	extern sendmsg_func sendmsg_f;

	//关闭
	typedef int (*close_func)(int fd);
	extern close_func close_f;

	typedef int (*fcntl_func)(int fd, int cmd, ... /* arg */ );
	extern fcntl_func fcntl_f;

	typedef int (*ioctl_func)(int fd, unsigned long request, ...);
	extern ioctl_func ioctl_f;

	typedef int (*getsockopt_func)(int sockfd, int level, int optname,void *optval, socklen_t *optlen);
	extern getsockopt_func getsockopt_f;

	typedef int (*setsockopt_func)(int sockfd, int level, int optname,const void *optval, socklen_t optlen);
	extern setsockopt_func setsockopt_f;

	int connect_with_timeout(int fd, const sockaddr* addr, socklen_t addrlen, uint32_t timeout_ms);
};

#endif

