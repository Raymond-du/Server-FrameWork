#include "mysqlStatement.h"
#include <mysql/mysql.h>
#include "../Logger.h"
#include "mysqlResultSet.h"
#include "resultset.h"

namespace raymond {

	static auto g_logger = RAYMOND_LOG_BYNAME("system");
	bool MysqlStatement::query(std::string sql) {
		return real_query(sql.c_str(), sql.size());
	}

	bool MysqlStatement::real_query(const char* sql, size_t	length) {
		RAYMOND_LOG_FMT_DEBUG(g_logger, "sql:%s", sql);
		if (mysql_real_query(m_mysql, sql, length)){
			RAYMOND_LOG_FMT_WARN(g_logger, mysql_error(m_mysql));
			return false;
		}
		return true;
	}

	ResultSet::ptr MysqlStatement::getResultSet() {
		MYSQL_RES* result = mysql_store_result(m_mysql);
		if (result == nullptr) {
			RAYMOND_LOG_FMT_WARN(g_logger, mysql_error(m_mysql));
			return nullptr;
		}
		return MysqlResultSet::ptr(new MysqlResultSet(result));
	}

	std::string MysqlStatement::getError() {
		return std::string(mysql_error(m_mysql));
	}

	int MysqlStatement::getErrno() {
		return mysql_errno(m_mysql);
	}
}
