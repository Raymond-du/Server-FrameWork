#ifndef __UTIL_SQLUTIL_H
#define __UTIL_SQLUTIL_H

#include "../sql/mysqlConnection.h"
#include "../config.h"

namespace raymond {
	static auto sqlHost = Config::lookUp("mysql.host",
													std::string("127.0.0.1"), "数据库的主机地址");
	static auto sqlUser = Config::lookUp("mysql.username", 
													std::string("raymond"), "数据库的用户名");
	static auto sqlPasswd = Config::lookUp("mysql.passwd",
													std::string("697193du"), "数据库用户密码");
	static auto sqlDatabase = Config::lookUp("mysql.database",
													std::string("raymond"), "数据库名");
	static auto sqlPort = Config::lookUp("mysql.port",
													(int)3306, "数据库的端口号");

	Connection* getConnection();
//	{
//		static MysqlConnection g_connection;
//		if (!g_connection.isConnected()) {
//			g_connection.connect(sqlHost->getValue(),
//													sqlUser->getValue(),
//													sqlPasswd->getValue(),
//													sqlDatabase->getValue(),
//													sqlPort->getValue());
//		}
//		return &g_connection;
//	}
}

#endif
