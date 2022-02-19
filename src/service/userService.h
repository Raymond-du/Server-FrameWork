#ifndef __SERVICE_USERSERVICE_H
#define __SERVICE_USERSERVICE_H

#include "../entity/userBean.h"
#include <list>
#include "../util/sqlutil.h"

namespace raymond {
	class UserService {
	
	private:
		Connection* m_connection;
		std::string m_error;
	public:
		typedef std::shared_ptr<UserService> ptr;
		UserService();

		std::string getError() { return m_error;}
		User::ptr getUserById(const std::string& id);
		bool addUser(User& user);
		bool deleteUserById(const std::string& id);
		bool getAllUsers(std::list<User::ptr>& userList);
		/**
		 * @brief getPageUsers 获取一页的用户信息
		 *
		 * @param userList 返回用户的list
		 * @param offset 用户信息的偏移量
		 * @param pageSie 一页的用户的数量
		 *
		 * @return 
		 */
		bool getPageUsers(std::list<User::ptr>& userList, int offset, int pageSize);
		bool changeUser(const std::string& userId, User::ptr newUser);
	};
}

#endif
