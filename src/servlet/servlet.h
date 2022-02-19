#ifndef __SERVLET_SERVLET_H
#define __SERVLET_SERVLET_H
#include <bits/stdint-intn.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "EventType.h"
#include "../socket.h"
#include "../byteArray.h"
#include "../json/json/json.h"

namespace raymond {
	typedef Json::Value J_Value;
	// J_Value 的只能指针的表示
	//typedef std::shared_ptr<J_Value> J_ValuePtr;

	static int ReqEvent2Int(ReqEvent event) {
		return 1;
	}

	class Servlet {
	
	private:
		std::string m_name;

	public:
		typedef std::shared_ptr<Servlet> ptr;
		Servlet(const std::string& name) : m_name(name) {
		}
		virtual ~Servlet() {}

		virtual int32_t handle(Json::Value& request, Json::Value& response) = 0;
		std::string getName() { return m_name;}
	};

	class LoginServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<LoginServlet> ptr;
		LoginServlet() : Servlet("loginServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class RegistServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<RegistServlet> ptr;
		RegistServlet() : Servlet("RegistServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class GetMajorsInfoServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<GetMajorsInfoServlet> ptr;
		GetMajorsInfoServlet() : Servlet("GetUsersInfoServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class GetUsersInfoServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<GetUsersInfoServlet> ptr;
		GetUsersInfoServlet() : Servlet("GetUsersInfoServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class GetClzScheInfoServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<GetClzScheInfoServlet> ptr;
		GetClzScheInfoServlet() : Servlet("GetClzScheInfoServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class DelClzScheServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<DelClzScheServlet> ptr;
		DelClzScheServlet() : Servlet("DelClzScheServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class DelUserServlet : public Servlet {
	
	public:
		typedef std::shared_ptr<DelUserServlet> ptr;
		DelUserServlet() : Servlet("DelUserServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class ClzScheChangeServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<ClzScheChangeServlet> ptr;
		ClzScheChangeServlet() : Servlet("ClzScheChangeServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class AddCourseServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<AddCourseServlet> ptr;
		AddCourseServlet() : Servlet("AddCourseServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class UserChangeServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<UserChangeServlet> ptr;
		UserChangeServlet() : Servlet("UserChangeServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};

	class AddUserServlet : public Servlet {
		
	public:
		typedef std::shared_ptr<AddUserServlet> ptr;
		AddUserServlet() : Servlet("AddUserServlet") {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	};
	
	class DispatchServlet : public Servlet {
	
	public:
		typedef std::shared_ptr<DispatchServlet> ptr;
		typedef std::unordered_map<ReqEvent, Servlet::ptr> ServletMap;
		
		DispatchServlet();

		bool addServlet(ReqEvent event, Servlet::ptr servlet, bool autoreplace = true);
		bool addServlet(ReqEvent event, Servlet* pServlet, bool autoreplace = true);
		bool changeServlet(ReqEvent event, Servlet::ptr servlet);
		bool delServlet(ReqEvent event);

		int32_t handle(Json::Value& request, Json::Value& response) override;

	private:
		ServletMap m_map;
	};

	//拦截器  servlet主要用来判断用户是否登陆及权限
	/**
	 * @brief 进行请求是否正常  用户是否登陆
	 */
	class InterceptServlet : public Servlet {
	public:
		typedef std::shared_ptr<InterceptServlet> ptr;
		InterceptServlet() : Servlet("InterceptServlet"){
			m_servlet.reset(new DispatchServlet());
		}
		InterceptServlet(Servlet::ptr servlet) :
											Servlet("InterceptServlet") {
			m_servlet = servlet;	
		}
		~InterceptServlet() {}

		int32_t handle(Json::Value& request, Json::Value& response) override;
	private:
		Servlet::ptr m_servlet;

	};

	
	/**
	 * @brief 校验并解析数据成json格式
	 */
	class DataInteract {
	
	public:
		typedef std::shared_ptr<DataInteract> ptr;

		DataInteract() {}
		virtual ~DataInteract() {}

		//接受到的数据和发送的数据
		int32_t handle(SocketImpl::ptr sock);

	private:
		InterceptServlet m_servlet;
	};
	
}

#endif
