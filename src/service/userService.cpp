#include "userService.h"
#include <cstring>
#include <exception>
#include "../Logger.h"
#include <string>
#include <sys/socket.h>

namespace raymond {
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	UserService::UserService() {
		m_connection = getConnection();	
	}

	User::ptr UserService::getUserById(const std::string& id) {
		//auto connection = getConnection();
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return nullptr;
		}
		auto statement = m_connection->createStatement();

		std::string sql;
		sql.append("select Uname,Uage,Uphone,Uemail,Upasswd,Ujob_title,MNum,Uprivilege,Unotes ");
		sql.append("from user where user.Uid = \'");
		sql.append(id);
		sql.append("\' limit 1");
		if (false == statement->query(sql)) {
			return nullptr;
		}
		auto resultset = statement->getResultSet();
		if (resultset->nextResult() == false) {
			return nullptr;
		}
		User::ptr user(new User());
		try {
			user->setId(id);
			user->setName(resultset->getString(0));
			user->setAge(resultset->getInt(1));
			user->setPhone(resultset->getString(2));
			user->setEmail(resultset->getString(3));
			user->setPasswd(resultset->getString(4));
			user->setJobTitle(resultset->getString(5));
			user->setMajorNum(resultset->getInt(6));
			user->setPrivilege(resultset->getInt(7));
			user->setNote(resultset->getString(8));
		} catch (std::exception& e) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "getUserById error: %s", e.what());
			return nullptr;
		}

		return user;
	}

	//TODO: 这里需要添加多一点的验证
	bool UserService::addUser(User& user) {
		if (user.getId().empty() || user.getName().empty() 
					||user.getPasswd().empty() || user.getPhone().empty()) {
			m_error = "id,name,passwd,phone 不可为空";
			return false;
		}
//		auto connection = getConnection();
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		//这个地方没有进行判断major 是否存在的语句    后续加上
		char sql[256];
		memset(sql, '\0', 256);
		snprintf(sql, 256, "insert into user(Uid,Uname,Uage,Uphone,Uemail,Upasswd,Ujob_title,MNum,Uprivilege,Unotes) \
											value(\'%s\',\"%s\",%d,\"%s\",\"%s\",\"%s\",\'%s\',%d,%d,\'%s\')",
											user.getId().c_str(), user.getName().c_str(), user.getAge(), user.getPhone().c_str(),
											user.getEmail().c_str(), user.getPasswd().c_str(), user.getJobTitle().c_str(),
											user.getMajorNum(), user.getPrivilege(), user.getNote().c_str());
		if (false == statement->query(std::string(sql))) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();

		return true;
	}

	bool UserService::deleteUserById(const std::string& id) {
		//auto connection = getConnection();
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		std::string sql = "delete from user where Uid= " + id;

		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool UserService::getAllUsers(std::list<User::ptr>& userList) {
		//auto connection = getConnection();
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		if (false == statement->query("select Uid,Uname,Uage,Uphone,Uemail,Upasswd,Ujob_title,MNum,Uprivilege,Unotes from user")) {
			m_error = statement->getError();
			return false;
		}
		auto resultset = statement->getResultSet();
		User::ptr user = nullptr;
		while (resultset->nextResult()) {
			user.reset(new User());
			user->setId(resultset->getString(0));
			user->setName(resultset->getString(1));
			user->setAge(resultset->getInt(2));
			user->setPhone(resultset->getString(3));
			user->setEmail(resultset->getString(4));
			user->setPasswd(resultset->getString(5));
			user->setJobTitle(resultset->getString(6));
			user->setMajorNum(resultset->getInt(7));
			user->setPrivilege(resultset->getInt(8));
			user->setNote(resultset->getString(9));

			RAYMOND_LOG_FMT_DEBUG(g_logger, "user:%s", user->toString().c_str());
			userList.push_back(user);
			user = nullptr;
		}
		return true;
	}
	
//	void strSplice(std::string& s1, 
//								const std::string& s2,
//								const std::string& delimiter) {
//		if (!s1.empty()) {
//			s1.append(delimiter);
//		}
//		s1.append(s2);
//	}

	//TODO: 这里之后需要添加对major的判断  可以在调用的时候判断
	bool UserService::changeUser(const std::string& userId, User::ptr newUser) {
	//	auto connection = getConnection();
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		
		std::string sql;
		sql.append("update user set ");
		if (!newUser->getId().empty()) {
			sql.append("Uid=\'").append(newUser->getId());
			sql.append("\',");
		}
		if (!newUser->getName().empty()) {
			sql.append("Uname=\'").append(newUser->getName());
			sql.append("\',");
		}
		if (newUser->getAge() != -1) {
			sql.append("Uage=\'").append(std::to_string(newUser->getAge()));
			sql.append("\',");
		}
		if (!newUser->getPhone().empty()) {
			sql.append("Uphone=\'").append(newUser->getPhone());
			sql.append("\',");
		}
		if (!newUser->getEmail().empty()) {
			sql.append("Uemail=\'").append(newUser->getEmail());
			sql.append("\',");
		}
		if (!newUser->getPasswd().empty()) {
			sql.append("Upasswd=\'").append(newUser->getPasswd());
			sql.append("\',");
		}
		if (!newUser->getJobTitle().empty()) {
			sql.append("Ujob_title=\'").append(newUser->getJobTitle());
			sql.append("\',");
		}
		if (newUser->getMajorNum() != -1) {
			sql.append("MNum=\'").append(std::to_string(newUser->getMajorNum()));
			sql.append("\',");
		}
		if (newUser->getPrivilege() != -1) {
			sql.append("Uprivilege=\'").append(std::to_string(newUser->getPrivilege()));
			sql.append("\',");
		}
		if (!newUser->getNote().empty()) {
			sql.append("Unotes=\'").append(newUser->getNote());
			sql.append("\',");
		}
		if (sql.back() != ',') {
			m_error = "该对象没有改变";
			return false;
		}
		sql.pop_back();
		sql.append(" where Uid=\'").append(userId).append("\'");
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool UserService::getPageUsers(std::list<User::ptr>& userList, int offset, int pageSize) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		if (pageSize > 30 || pageSize < 10) {
			pageSize = 30;
		}

		std::string sql = "select Uid,Uname,Uage,Uphone,Uemail,Upasswd,Ujob_title,MNum,Uprivilege,Unotes from user";
		sql.append(" limit ").append(std::to_string(pageSize));
		sql.append(" offset ").append(std::to_string(offset));
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		auto resultset = statement->getResultSet();
		User::ptr user = nullptr;
		while (resultset->nextResult()) {
			user.reset(new User());
			user->setId(resultset->getString(0));
			user->setName(resultset->getString(1));
			user->setAge(resultset->getInt(2));
			user->setPhone(resultset->getString(3));
			user->setEmail(resultset->getString(4));
			user->setPasswd(resultset->getString(5));
			user->setJobTitle(resultset->getString(6));
			user->setMajorNum(resultset->getInt(7));
			user->setPrivilege(resultset->getInt(8));
			user->setNote(resultset->getString(9));
			userList.push_back(user);
			user = nullptr;
		}
		return true;
	
	}
}
