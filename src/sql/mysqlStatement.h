#ifndef __SQL_MYSQLSTATEMENT_H
#define __SQL_MYSQLSTATEMENT_H

#include "statement.h"
#include "mysqlResultSet.h"
#include <memory>
#include <mysql/mysql.h>

namespace raymond {
	class MysqlConnection;
	class MysqlStatement : public Statement {
	friend class MysqlConnection;
	public:
		typedef std::shared_ptr<MysqlStatement> ptr;

		bool query(std::string sql) override;
		bool real_query(const char* sql, size_t length) override;
		ResultSet::ptr getResultSet() override;
		std::string getError() override;
		int getErrno() override;
	
	private:
		MYSQL* m_mysql = nullptr;
		bool m_error = false;

		MysqlStatement(MYSQL* mysql) : m_mysql(mysql) {}
	};
}

#endif
