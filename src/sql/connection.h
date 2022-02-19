#ifndef __SQL_CONNECTION_H
#define __SQL_CONNECTION_H

#include "statement.h"

namespace raymond {
	/**
	 * @brief 数据库的链接接口
	 */
	class Connection {
	public:
		/**
		 * @brief close 关闭此次链接
		 */
		virtual void close() = 0;
		/**
		 * @brief commit 使自上次提交/回滚以来所做的所有更改为永久行
		 */
		virtual void commit() = 0;
		/**
		 * @brief createStatement 创建一个对象,用于将sql语句发送到数据库中
		 *
		 * @return 返回statement对象
		 */
		virtual Statement::ptr createStatement() = 0;
		/**
		 * @brief isClose 检索connection对象是否已经关闭连接
		 *
		 * @return 
		 */
		virtual bool isClose() = 0;
		/**
		 * @brief isAutoCommit 当前是否是自动提交模式
		 *
		 * @return 
		 */
		virtual bool isAutoCommit() = 0;
		/**
		 * @brief rollBack 撤销当前事务所做的所有更改
		 */
		virtual void rollBack() = 0;
		/**
		 * @brief setAutoCommit 设置此链接的自动提交模式为给定状态
		 *
		 * @param autoCommit
		 */
		virtual void setAutoCommit(bool autoCommit) = 0;
	};
}

#endif

