#include "servlets.h"
#include "../service/userService.h"

namespace raymond {
	int32_t loginFunc(Json::Value& request, Json::Value& response) {
		if (request["id"].isNull() || request["passwd"].isNull()) {
			response["result"] = "error";
			response["error"] = "请求异常";
			return 0;
		}
		int userId = request["id"].asInt();
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

	int32_t registFunc(Json::Value& request, Json::Value& response) {
		if (request["id"].isNull() ||
				request["name"].isNull() ||
				request["passwd"].isNull() ||
				request["email"].isNull()) {
			response["result"] = "error";
			response["error"] = "请求有误,请重新输入";
			return 0;
		}
		User user(User(
					request["id"].asInt(),
					request["name"].asString(),
					request["passwd"].asString(),
					request["phone"].asString(),
					request["email"].isNull() ? "" : request["email"].asString()));

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
}
