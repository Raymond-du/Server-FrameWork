#include "application.h"
#include "address.h"
#include "iomanager.h"
#include "serverSocket.h"
#include "servlet/servlet.h"
#include "servlet/servlets.h"
#include "socket.h"
#include "socketImpl.h"
#include "json/json/value.h"
#include <bits/stdint-intn.h>
#include <functional>
#include <iostream>
#include "Logger.h"
#include "config.h"
#include <sys/socket.h>

namespace raymond {

	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	Application::Application() {
		m_IOManager.reset(new raymond::IOManager("socket", true, 3));
	}
		
	Application::Application(int argv, char** argc) :
													Application() {
		init();
	}
	
	void Application::init() {
		//启动配置文件
		Config::loadFromConFile("porperties.yml");
	}


	void on_recv_data(Socket::ptr sock) {
		DataInteract interact;
		interact.handle(sock);
	}

	void on_accept_client(Socket::ptr sock) {
		sock->setAsynReadEvent(on_recv_data);
	}

	void acceptEvent(ServerSocket::ptr server_socket) {
		auto client_socket = server_socket->Accept();
		RAYMOND_LOG_FMT_INFO(g_logger, "server accept : %s", client_socket->getRemoteAddr()->toString().c_str());
		if (client_socket != nullptr) {
			on_accept_client(client_socket);
		} else {
			RAYMOND_LOG_FMT_WARN(g_logger, "accept client failure");
		}
	}

	int Application::run() {
		m_IOManager->schedule([this]() {
				work_func();
			});	
		
		return 0;
	}

	void Application::work_func() {
			auto ioMan = IOManager::getIOManager();
			ServerSocket::ptr ser_socket(new ServerSocket(AF_INET));
			auto addr = IPv4Address::getIpv4Addr("0.0.0.0", 1214);
			if (ser_socket->Bind(addr) == false) {
				std::cout << "sock bind error" << std::endl;
				return;
			}
			ser_socket->setClientAutoConfig(true);
			ser_socket->setHookEnable(true);
			ser_socket->setNonBlock(true);
			ser_socket->Listen(5);
			ser_socket->setListenFun(acceptEvent);
	}

}
int main(int argv, char** argc) {
	raymond::Application app(argv, argc);
	app.run();
	return 0;
}
