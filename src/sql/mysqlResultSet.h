#ifndef __SQL_MYSQLRESULTSET_H
#define __SQL_MYSQLRESULTSET_H

#include <string>
#include <memory>
#include "resultset.h"
#include <mysql/mysql.h>
#include <vector>

namespace raymond {

	class MysqlResultSet : public ResultSet{
	private:
		MYSQL_RES* m_result = nullptr;
		MYSQL_ROW m_row = nullptr;
		MYSQL_FIELD* m_field = nullptr;
		unsigned int m_fieldCount = 0;
		unsigned int m_rowsNum = 0;
	public:
		typedef std::shared_ptr<MysqlResultSet> ptr;
		typedef std::vector<std::string> stringvector;
		MysqlResultSet(MYSQL_RES* result);
		~MysqlResultSet();

		bool getBool(int columnIndex) override;
		bool getBool(const std::string& columnLable) override;
		int getInt(int columnIndex) override;
		int getInt(const std::string& columnLable) override;
		float getFloat(int columnIndex) override;
		float getFloat(const std::string& columnLable) override;
		long getLong(int columnIndex) override;
		long getLong(const std::string& columnLable) override;
		double getDouble(int columnIndex) override;
		double getDouble(const std::string& columnLable) override;
		std::string getString(int columnIndex) override;
		std::string getString(const std::string& columnLable) override;
		Byte::ptr getBytes(int columnIndex) override;
		Byte::ptr getBytes(const std::string& columnLable) override;
		char* getRawData(int columnIndex, int* length) override;
		char* getRawData(const std::string& columnLable, int* length) override;
		Time::ptr getTime(int columnIndex) override;
		Time::ptr getTime(const std::string& columnLable) override;
		Date::ptr getDate(int columnIndex) override;
		Date::ptr getDate(const std::string& columnLable) override;
		DateTime::ptr getDateTime(int columnIndex) override;
		DateTime::ptr getDateTime(const std::string& columnLable) override;

		unsigned int getRowsNum() { return m_rowsNum;}
		unsigned int getFieldCount() { return m_fieldCount;}
		//获取属性的数组
		unsigned int getFieldVector(stringvector& strs);
		bool nextResult() override;
	};
}

#endif
