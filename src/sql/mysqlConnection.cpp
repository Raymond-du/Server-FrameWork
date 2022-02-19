#include "mysqlConnection.h"
#include "mysqlStatement.h"
#include <mysql/mysql.h>
#include "../Logger.h"
#include "../macro.h"

namespace raymond {
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	MysqlConnection::~MysqlConnection() {
		if (!isClose()) {
			mysql_close(m_mysql);
		}
	}

	unsigned int MysqlConnection::Errno() {
		return mysql_errno(m_mysql);
	}
	
	MysqlConnection::MysqlConnection() {
		m_mysql = mysql_init(nullptr);
		mysql_set_character_set(m_mysql, "utf8");
	}

	MysqlConnection::MysqlConnection(const std::string& host,
										const std::string& user,
										const std::string& passwd,
										const std::string& database,
										unsigned int port) {
		m_mysql = mysql_init(nullptr);
		mysql_set_character_set(m_mysql, "utf8");
		if (!mysql_real_connect(m_mysql, host.c_str(), user.c_str(),
																passwd.c_str(), database.c_str(),
																port, nullptr, 0)) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "未能和数据库建立连接");
			return;
		}
		m_connected = true;
	}

	bool MysqlConnection::connect(const std::string& host,
															const std::string& user,
															const std::string& passwd,
															const std::string& database,
															unsigned int port) {
		auto rt = mysql_real_connect(m_mysql, host.c_str(), user.c_str(),
																passwd.c_str(), database.c_str(),
																port, nullptr, 0);
		if (rt == nullptr){
			RAYMOND_LOG_FMT_ERROR(g_logger, "未能和数据库建立连接");
			return false;
		}
		m_connected = true;
		return true;
	}

	void MysqlConnection::close() {
		mysql_close(m_mysql);
	}

	void MysqlConnection::commit() {
		mysql_commit(m_mysql);	
	}

	bool MysqlConnection::isClose() {
		return mysql_ping(m_mysql) == 0 ? false : true;
	}

	bool MysqlConnection::isAutoCommit() {
		return m_autoCommit;
	}

	void MysqlConnection::rollBack() {
		if (mysql_rollback(m_mysql)) {
			RAYMOND_LOG_FMT_WARN(g_logger, "error when rollbacking");
		}
	}

	void MysqlConnection::setAutoCommit(bool autoCommit) {
		if (0 == mysql_autocommit(m_mysql, autoCommit)) {
			m_autoCommit = autoCommit;
		} else {
			RAYMOND_LOG_FMT_WARN(g_logger, "error when setting mysql autocommit");
		}
	}

	Statement::ptr MysqlConnection::createStatement() {
		if (m_mysql == nullptr) {
			return nullptr;
		}
		return MysqlStatement::ptr(new MysqlStatement(m_mysql));
	}
}
