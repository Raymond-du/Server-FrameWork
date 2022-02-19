#include "mysqlResultSet.h"
#include <cstring>
#include <mysql/mysql.h>
#include "../Logger.h"
#include "resultset.h"

namespace raymond {

	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	MysqlResultSet::MysqlResultSet(MYSQL_RES* result)
																:m_result(result) {
		m_rowsNum = mysql_num_rows(m_result);
		m_fieldCount = mysql_num_fields(m_result);
		m_field = mysql_fetch_fields(m_result);
	}
	
	MysqlResultSet::~MysqlResultSet() {
		mysql_free_result(m_result);
	}

	bool MysqlResultSet::getBool(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr || strcmp(m_row[columnIndex], "0") == 0) {
				return false;
			}
			return true;
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return false;
	}

	bool MysqlResultSet::getBool(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getBool(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return false;
	}

	int MysqlResultSet::getInt(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return 0;
			}
			return atoi(m_row[columnIndex]);
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return 0;
	}
	
	int MysqlResultSet::getInt(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getInt(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return 0;
	}
	
	float MysqlResultSet::getFloat(int columnIndex) {
		return getDouble(columnIndex);
	}
	
	float MysqlResultSet::getFloat(const std::string& columnLable) {
		return getDouble(columnLable);
	}
	
	long MysqlResultSet::getLong(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return 0;
			}
			return atol(m_row[columnIndex]);
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return 0;
	}
	
	long MysqlResultSet::getLong(const std::string& columnLable) {
	
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getLong(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return 0;
	}
	
	double MysqlResultSet::getDouble(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return 0;
			}
			return atof(m_row[columnIndex]);
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return 0;
	}

	double MysqlResultSet::getDouble(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getDouble(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return 0;
	}

	std::string MysqlResultSet::getString(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return "";
			}
			return std::string(m_row[columnIndex]);
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return "";
	}
	
	std::string MysqlResultSet::getString(const std::string& columnLable) {
	
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getString(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return "";
	}
	
	Byte::ptr MysqlResultSet::getBytes(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			unsigned long* lengths = mysql_fetch_lengths(m_result);
			if (m_row[columnIndex] == nullptr) {
				return nullptr;
			}
			return Byte::ptr(new Byte(m_row[columnIndex], lengths[columnIndex]));
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return nullptr;
	}
	
	Byte::ptr MysqlResultSet::getBytes(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getBytes(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return nullptr;
	}

	char* MysqlResultSet::getRawData(int columnIndex, int* length) {
		if (columnIndex < m_fieldCount) {
			unsigned long* lengths = mysql_fetch_lengths(m_result);
			*length = lengths[columnIndex];
			return m_row[columnIndex];
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return nullptr;
		
	}

	char* MysqlResultSet::getRawData(const std::string& columnLable, int* length) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getRawData(i, length);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return nullptr;
	
	}

	Time::ptr MysqlResultSet::getTime(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return nullptr;
			}
			return Time::ptr(new Time(m_row[columnIndex]));
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return nullptr;
	
	}
	
	Time::ptr MysqlResultSet::getTime(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getTime(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return nullptr;
	
	}
	
	Date::ptr MysqlResultSet::getDate(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return nullptr;
			}
			return Date::ptr(new Date(m_row[columnIndex]));
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return nullptr;
	
	}
	
	Date::ptr MysqlResultSet::getDate(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getDate(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return nullptr;
	}
	
	DateTime::ptr MysqlResultSet::getDateTime(int columnIndex) {
		if (columnIndex < m_fieldCount) {
			if (m_row[columnIndex] == nullptr) {
				return nullptr;
			}
			return DateTime::ptr(new DateTime(m_row[columnIndex]));
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "columnIndex over fieldCount");
		return nullptr;
	}
	
	DateTime::ptr MysqlResultSet::getDateTime(const std::string& columnLable) {
		for (int i = 0; i < m_fieldCount; i++) {
			if (strcmp(columnLable.c_str(), m_field[i].name) == 0) {
				return getDateTime(i);
			}
		}
		RAYMOND_LOG_FMT_WARN(g_logger, "can't find column which is named %s",
												columnLable.c_str());
		return nullptr;
	}

	bool MysqlResultSet::nextResult() {
		m_row = mysql_fetch_row(m_result);
		return m_row != nullptr;
	}

	unsigned int MysqlResultSet::getFieldVector(stringvector& strs) {
		for (int i = 0; i < m_fieldCount; i++) {
			strs.push_back(std::string(m_field[i].name));
		}
		return m_fieldCount;
	}
}
