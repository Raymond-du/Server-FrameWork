#ifndef __SQL_MYSQLCONNECTION_H
#define __SQL_MYSQLCONNECTION_H

#include "connection.h"
#include <mysql/mysql.h>
#include <string>

namespace raymond {
	class MysqlConnection : public Connection {
	private:
		MYSQL* m_mysql = nullptr;
		bool m_autoCommit = true;
		bool m_connected = false;
	public:
		MysqlConnection();
		~MysqlConnection();
		/**
		 * @brief MysqlConnection 
		 *
		 * @param host 主机地址
		 * @param user	用户名
		 * @param passwd	密码
		 * @param database	数据名称
		 * @param port	端口地址
		 */
		MysqlConnection(const std::string& host,
										const std::string& user,
										const std::string& passwd,
										const std::string& database,
										unsigned int port = 3306);

		unsigned int Errno();

		bool connect(const std::string& host,
								const std::string& user,
								const std::string& passwd,
								const std::string& database,
								unsigned int port = 3306);

		/**
		 * @brief isConnected 返回是否已经连接到数据库服务器
		 *
		 * @return 
		 */
		bool isConnected() { return m_connected;}
		/**
		 * @brief close 关闭数据库连接
		 */
		void close();
		/**
		 * @brief commit 提交事务
		 */
		void commit();
		/**
		 * @brief createStatement 创建一个statement对象用于执行查询语句
		 *
		 * @return 
		 */
		Statement::ptr createStatement();
		/**
		 * @brief isClose 是否连接已断开
		 *
		 * @return 
		 */
		bool isClose();
		/**
		 * @brief isAutoCommit 是否式自动连接
		 *
		 * @return 
		 */
		bool isAutoCommit();
		/**
		 * @brief rollBack 回滚事务
		 */
		void rollBack();
		void setAutoCommit(bool autoCommit);
	};
}

#endif
