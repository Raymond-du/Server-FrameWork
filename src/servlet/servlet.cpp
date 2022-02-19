#include "servlet.h"
#include <algorithm>
#include <bits/stdint-intn.h>
#include <functional>
#include <list>
#include <memory>
#include <netdb.h>
#include <string>
#include "../entity/DepartmentBean.h"
#include "../entity/MajorBean.h"
#include "../Logger.h"
#include "../hook.h"
#include "../json/json/json.h"
#include "../service/userService.h"
#include "../service/TimeTableService.h"

namespace raymond {

	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	int32_t LoginServlet::handle(Json::Value& request, Json::Value& response) {
	
		if (request["account"].isNull() || request["passwd"].isNull()) {
			response["result"] = "error";
			response["error"] = "请求异常";
			return 0;
		}
		auto userId = request["account"].asString();
		auto passwd = request["passwd"].asString();
		UserService service;
		auto user = service.getUserById(userId);
		if (user) {
			if (passwd.compare(user->getPasswd()) == 0) {
				response["result"] = "true";
				response["message"] = "登录成功";
				return 1;
			} else {
				response["result"] = "error";
				response["error"] = "密码存在错误";
				return 0;
			}
		} else {
			response["result"] = "error";
			response["error"] = "不存在该用户";
			return 0;
		}
	}
	
	//TODO 缺少验证
	int32_t RegistServlet::handle(Json::Value& request, Json::Value& response) {
	
		if (request["account"].isNull() ||
				request["name"].isNull() ||
				request["passwd"].isNull() ||
				request["email"].isNull()) {
			response["result"] = "error";
			response["error"] = "请求有误,请重新输入";
			return 0;
		}

		User user(
					request["account"].asString(),
					request["age"].asInt(),
					request["privilege"].asInt(),
					request["name"].asString(),
					request["passwd"].asString(),
					request["phone"].asString(),
					request["email"].asString(),
				  request["jobTitle"].asString(),
					request["majorNum"].asInt(),
					request["note"].asString());

		UserService service;
		if (!service.addUser(user)) {
			response["result"] = "error";
			response["error"] = "数据添加时发生错误";
			return 0;
		}
		response["result"] = "true";
		response["message"] = "添加用户成功";
		return 1;
	}
	
	/**
	 *
		{
			departs: 
				[
					{departId:"", departName:""},
					{departId:"", departName:""}
				]
			majors:
				[
					{MajorId:"", MajorName:"", DNum:"", version : ["541_2019","541_2018"]},
					{MajorId:"", MajorName:"", DNum:"", version : ["551_2018"]}
				]
		}
	 */
	int32_t GetMajorsInfoServlet::handle(Json::Value& request, Json::Value& response) {
		auto& majorList = MajorManager::getInstance()->getAll();
		auto& departList = DepartmentManager::getInstance()->getAll();
		for (auto& depart : departList) {
			Json::Value departJson;
			departJson["departId"] = depart.second->getNum();
			departJson["departName"] = depart.second->getName();
			response["departs"].append(departJson);
		}
		for (auto& _major : majorList) {
			Json::Value majorJson;
			majorJson["majorId"] = _major.second->getNum();
			majorJson["majorName"] = _major.second->getName();
			majorJson["DNum"] = _major.second->getDeparNum();
			majorJson["version"] = _major.second->versionToJson();
			response["majors"].append(majorJson);
		}
		return true;
	}

	/**
	 * 发送用户的信息
	 * {"users":[{id:"",age:""...},{id:"",age:""...}]}
	 */
	int32_t GetUsersInfoServlet::handle(Json::Value& request, Json::Value& response) {
		UserService userService;
		std::list<User::ptr> usersList;
		//获取请求的信息 偏移量和获取的个数
		int offset = 0, length = 0;
		if (request.isMember("offset")) {
			offset = request["offset"].asInt();
		}
		if (request.isMember("length")) {
			length = request["length"].asInt();
		}

		userService.getPageUsers(usersList, offset, length);
		for (auto& user : usersList) {
			Json::Value userJson;
			userJson["id"] = user->getId();
			userJson["age"] = user->getAge();
			userJson["privilege"] = user->getPrivilege();
			userJson["name"] = user->getName();
			userJson["passwd"] = user->getPasswd();
			userJson["phone"] = user->getPhone();
			userJson["email"] = user->getEmail();
			userJson["jobTitle"] = user->getJobTitle();
			userJson["majorName"] = user->getMajorName();
			userJson["note"] = user->getNote();
			response["users"].append(userJson);
		}
		return 0;
	}

	int32_t GetClzScheInfoServlet::handle(Json::Value& request, Json::Value& response) {
		if (!request.isMember("tableName")) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "request has not tableName");
			return -1;
		}
		std::string tableName = request["tableName"].asString();
		TimeTableService service;
		auto timetable = service.getTimeTable(tableName);
		if (timetable == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "can find tableName :%s", tableName.c_str());
			return -1;
		}
		for (auto& course : timetable->getCourses()) {
			Json::Value courseJson;
			courseJson["TNum"] = course->getTNum();
			courseJson["TName"] = course->getTName();
			courseJson["TEngName"] = course->getEngName();
			courseJson["TCridet"] = course->getTCridet();
			courseJson["TTrainMode"] = course->getTTrainMode();
			courseJson["TProperty"] = course->getTProperty();
			courseJson["IsEnabled"] = course->getIsEnabled();
			courseJson["TSubmitter"] = course->getTSubmitter();
			courseJson["TAuditor"] = course->getTAuditor();
			courseJson["TPeriod"] = course->getTPeriod();
			courseJson["TOfferSem"] = course->getTOfferSemester();
			courseJson["TTeachWeak"] = course->getTTeachWeak();
			courseJson["TTestMode"] = course->getTestMode();
			courseJson["TOfferedDName"] = course->getTOfferedDName();
			courseJson["IsMajor"] = course->getIsMajor();
			courseJson["IsKernel"] = course->getIsKernel();
			courseJson["TNote"] = course->getTNote();
			courseJson["TOprtPerm"] = course->getOprtPermission();
			courseJson["TIsInClass"] = course->getTIsInClass();
			courseJson["TCourseType"] = course->getTCourseType();
			response["TimeTable"].append(courseJson);
		}
		RAYMOND_LOG_FMT_DEBUG(g_logger, "send TimeTable %d", timetable->getCourses().size());

		return 0;
	}

	int32_t DelClzScheServlet::handle(Json::Value& request, Json::Value& response) {
		if (!request.isMember("params")) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "request has not params");
			return -1;
		}
		std::string tablename = request["tableName"].asString();
		TimeTableService service;
		for (auto i : request["params"]) {
			RAYMOND_LOG_FMT_DEBUG(g_logger, "del timetable %d", i.asInt());
			if (false == service.delCourse(tablename, i.asInt())) {
				response["result"] = "false";
				response["reason"] = service.getError();
				return -1;
			}
		}
		response["result"] = "true";
		response["message"] = "删除成功";
		return 0;
	}

	int32_t AddCourseServlet::handle(Json::Value& request, Json::Value& response) {
		CourseBean::ptr course(new CourseBean());
		TimeTableService service;
		std::string tablename = request["tableName"].asString();
		course->setTCourseType(request["CType"].asString());
		course->setTProperty(request["CProperty"].asString());
		course->setTNum(request["CNum"].asInt());
		course->setTName(request["CName"].asString());
		course->setTCridet(request["CCredit"].asInt());
		course->setTPeriod(request["CPeriod"].asInt());
		course->setTOfferSemester(request["COfferSem"].asInt());
		course->setTTeachWeak(request["CTeachWeak"].asInt());
		course->setIsKernel(request["CIskenel"].asInt());
		course->setTOfferedDName(request["COfferMajor"].asString());
		course->setIsInClass(request["CIsinclass"].asInt());

		if (false == service.addCourse(tablename, course)) {
			response["result"] = "false";
			response["reason"] = service.getError();
			return -1;
		}
	
		response["result"] = "true";
		response["message"] = "添加成功";
		return 0;

	}

	int32_t DelUserServlet::handle(Json::Value& request, Json::Value& response) {
		UserService service;
		for (auto i : request["params"]) {
			if (false == service.deleteUserById(i.as<std::string>())) {
				response["result"] = "false";
				response["reason"] = service.getError();
				return -1;
			}
		}
		response["result"] = "true";
		response["message"] = "删除成功";
		return 0;
	}

	int32_t UserChangeServlet::handle(Json::Value& request, Json::Value& response) {
		if (!request.isMember("params")) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "request has not params");
			return -1;
		}
		User::ptr changeUser(new User());
		UserService service;
		Json::Value::Members mem = request["params"].getMemberNames();
		for (auto it = mem.begin(); it != mem.end(); it ++) {
			Json::Value tmp = request["params"][*it];
			if (tmp.isMember("名字")) {
				changeUser->setName(tmp["名字"].asString());
			}
			if (tmp.isMember("年龄")) {
				changeUser->setAge(std::stoi(tmp["年龄"].asString()));
			}
			if (tmp.isMember("权限")) {
				changeUser->setPrivilege(std::stoi(tmp["权限"].asString()));
			}
			if (tmp.isMember("密码")) {
				changeUser->setPasswd(tmp["密码"].asString());
			}
			if (tmp.isMember("电话")) {
				changeUser->setPhone(tmp["电话"].asString());
			}
			if (tmp.isMember("邮箱")) {
				changeUser->setEmail(tmp["邮箱"].asString());
			}
			if (tmp.isMember("职称")) {
				changeUser->setJobTitle(tmp["职称"].asString());
			}
			if (tmp.isMember("所属专业")) {
				changeUser->setMajorName(tmp["所属专业"].asString());
			}
			if (tmp.isMember("备注")) {
				changeUser->setNote(tmp["备注"].asString());
			}
			if (false == service.changeUser(*it, changeUser)) {
				response["result"] = "false";
				response["reason"] = service.getError();
				return -1;
			}
		}
		response["result"] = "true";
		response["message"] = "删除成功";
		return 0;
	}

	int32_t AddUserServlet::handle(Json::Value& request, Json::Value& response) {
		User user;
		UserService service;
		
		user.setId(request["Uid"].asString());
		user.setName(request["Uname"].asString());
		user.setAge(request["Uage"].asInt());
		user.setPrivilege(request["Uprivilege"].asInt());
		user.setPasswd(request["Upasswd"].asString());
		user.setPhone(request["Uphone"].asString());
		user.setEmail(request["Uemail"].asString());
		user.setJobTitle(request["Ujobtitle"].asString());
		user.setMajorName(request["UmajorName"].asString());
		user.setNote(request["Unote"].asString());

		if (false == service.addUser(user)) {
			response["result"] = "false";
			response["reason"] = service.getError();
			return -1;
		}
	
		response["result"] = "true";
		response["message"] = "添加成功";
		return 0;
		
	}
	
	int32_t ClzScheChangeServlet::handle(Json::Value& request, Json::Value& response) {
		if (!request.isMember("params")) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "request has not params");
			return -1;
		}
		std::string tablename = request["tableName"].asString();
		TimeTableService service;
		CourseBean::ptr course(new CourseBean());
		Json::Value::Members mem = request["params"].getMemberNames();
		for (auto it = mem.begin(); it != mem.end(); it++) {
			Json::Value tmp = request["params"][*it];
			if (tmp.isMember("课程类别")) {
				course->setTCourseType(tmp["课程类别"].asString());
			}
			if (tmp.isMember("课程属性")) {
				course->setTProperty(tmp["课程属性"].asString());
			}
			if (tmp.isMember("课程名称")) {
				course->setTName(tmp["课程名称"].asString());
			}
			if (tmp.isMember("学分")) {
				course->setTCridet(std::stoi(tmp["学分"].asString()));
			}
			if (tmp.isMember("开设学期")) {
				course->setTOfferSemester(std::stoi(tmp["开设学期"].asString()));
			}
			if (tmp.isMember("教学周数")) {
				course->setTTeachWeak(std::stoi(tmp["教学周数"].asString()));
			}
			if (tmp.isMember("考核方式")) {
				course->setTestMode(tmp["考核方式"].asString());
			}
			if (tmp.isMember("开课系部")) {
				course->setTOfferedDName(tmp["开课系部"].asString());
			}
			if (false == service.changeCourse(tablename, std::stoi(*it), course)) {
				response["result"] = "false";
				response["reason"] = service.getError();
				return -1;
			}
		}
		response["result"] = "true";
		response["message"] = "删除成功";
		return 0;
	}

	int32_t InterceptServlet::handle(Json::Value& request, Json::Value& response) {
		//进行拦截器进行拦截 下面进行处理
	
		return m_servlet->handle(request, response);
	}
	
	DispatchServlet::DispatchServlet() : Servlet("DispatchServlet") {
		addServlet(ReqEvent::UserLoginEvent, new LoginServlet());
		addServlet(ReqEvent::UserRegistEvent, new RegistServlet());
		addServlet(ReqEvent::GetMajorsInfoEvent, new GetMajorsInfoServlet());
		addServlet(ReqEvent::GetUsersInfoEvent, new GetUsersInfoServlet());
		addServlet(ReqEvent::UserChangeEvent, new UserChangeServlet());
		addServlet(ReqEvent::GetClzScheInfoEvent, new GetClzScheInfoServlet());
		addServlet(ReqEvent::DelUserEvent, new DelUserServlet());
		addServlet(ReqEvent::DelClzScheEvent, new DelClzScheServlet());
		addServlet(ReqEvent::ClzScheChangeEvent, new ClzScheChangeServlet());
		addServlet(ReqEvent::AddUserEvent, new AddUserServlet());
		addServlet(ReqEvent::AddCourseEvent, new AddCourseServlet());
	}

	bool DispatchServlet::addServlet(ReqEvent event, Servlet::ptr servlet, bool autoreplace) {
		if (autoreplace) {
			m_map[event] = servlet;
			return true;
		} else {
			auto it = m_map.find(event);
			if (it == m_map.end()) {
				m_map[event] = servlet;
				return true;
			} else {
				return false;
			}
		}
	}
	
	bool DispatchServlet::addServlet(ReqEvent event, Servlet* pServlet, bool autoreplace) {
		Servlet::ptr servlet(pServlet);
		return addServlet(event, servlet, autoreplace);
	}

	bool DispatchServlet::changeServlet(ReqEvent event, Servlet::ptr servlet) {
		auto it = m_map.find(event);
		if (it == m_map.end()) {
			m_map[event] = servlet;
			return true;
		}
		it->second = servlet;
		return true;
	}

	bool DispatchServlet::delServlet(ReqEvent event) {
		auto it = m_map.find(event);
		if (it != m_map.end()) {
			m_map.erase(it);
			return true;
		}
		return false;
	}

	int32_t DispatchServlet::handle(Json::Value& request, Json::Value& response) {
		ReqEvent event = ReqEvent::UnkownEvent;

		//isInt一直返回错误
		if (!request["commandId"].isNull()) {
			RAYMOND_LOG_FMT_INFO(g_logger, "请求id %d", request["commandId"].asInt());

			event = static_cast<ReqEvent>(request["commandId"].asInt());
			auto it = m_map.find(event);
			if (it != m_map.end()) {
				return it->second->handle(request, response);
			}
		}
		return -1;
	}

	//applicatin中 读信息的function
	int32_t DataInteract::handle(SocketImpl::ptr sock) {
		Socket::ptr socket = std::dynamic_pointer_cast<Socket>(sock);
		auto& buf = socket->getOutputArray();
		size_t len = buf.getSize();
		char* buffer = new char[len];
		memset(buffer, 0, len);
		buf.read(buffer, len);

		//test
		RAYMOND_LOG_FMT_INFO(g_logger, "接受客户端信息: %s", buffer);	

		Json::CharReaderBuilder readerBuild;
		std::unique_ptr<Json::CharReader> reader(readerBuild.newCharReader());
		Json::Value request; 
		Json::Value response;
		JSONCPP_STRING err;
		reader->parse(buffer, buffer + len, &request, &err);	
		m_servlet.handle(request, response);
		std::string resp = response.toStyledString();
		std::cout << resp.size() << std::endl;;

		RAYMOND_LOG_FMT_INFO(g_logger, "发送到客户端信息: %s", resp.c_str());
		socket->Send((char*)resp.c_str(), resp.size());

		delete[] buffer;
		return 0;
	}
}
