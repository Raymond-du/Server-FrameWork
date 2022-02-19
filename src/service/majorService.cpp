#include "majorService.h"
#include "../entity/MajorBean.h"
#include "../util/sqlutil.h"
#include <cstdio>
#include <cstring>
#include <exception>
#include "../Logger.h"
#include <string>
#include "../json/json/json.h"

namespace raymond {
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	MajorService::MajorService() {
		m_connection = getConnection();
	}

	MajorBean::ptr MajorService::getMajorByNum(int num) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return nullptr;
		}
		auto statement = m_connection->createStatement();
		std::string sql;
		sql.append("select MName,DNum,TableVersion from major where MNum = ");
		sql.append(std::to_string(num));
		sql.append(" limit 1");
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return nullptr;
		}
		auto resultset = statement->getResultSet();
		if (resultset->nextResult() == false) {
			m_error = "cant not find this data";
			return nullptr;
		}
		MajorBean::ptr _major(new MajorBean());
		try {
			_major->setNum(num);
			_major->setName(resultset->getString(0));
			_major->setDepartNum(resultset->getInt(1));
			//这里进行json解析
			std::string jsonStr = resultset->getString(22);
			
			if (!_major->setVersion(jsonStr)) {
				RAYMOND_LOG_FMT_ERROR(g_logger, "获取major version 失败,字段不为数组");
			} 
		} catch (std::exception& e) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "getMajorByNum error: %s", e.what());
			return nullptr;
		}
		return _major;
	}

	bool MajorService::addMajor(MajorBean::ptr major) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		char sql[256];
		snprintf(sql, 256, "insert into major(MNum,MName,DNum,TableVersion) value(%d,\"%s\",%d,\"%s\")", major->getNum(), 
				major->getName().c_str(), major->getDeparNum(), major->versionToJson().toStyledString().c_str());
		if (false == statement->query(std::string(sql))) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}
	
	bool MajorService::deleteMajorByNum(int num) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		std::string sql = "delete from major where MNum=" + std::to_string(num);
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}
	
	bool MajorService::getAllMajors(std::list<MajorBean::ptr>& majorList) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		if (false == statement->query("select MNum,MName,DNum,TableVersion from major")) {
			m_error = statement->getError();
			return false;
		}
		auto resultset = statement->getResultSet();
		MajorBean::ptr _major = nullptr;
		while (resultset->nextResult()) {
			try {
				_major.reset(new MajorBean());
				_major->setNum(resultset->getInt(0));
				_major->setName(resultset->getString(1));
				_major->setDepartNum(resultset->getInt(2));

				std::string vJson = resultset->getString(3);
				if (!_major->setVersion(vJson)) {
					RAYMOND_LOG_FMT_ERROR(g_logger, "获取major version 失败,字段不为数组");
				} 
			} catch (std::exception& e) {
				RAYMOND_LOG_FMT_ERROR(g_logger, "getAllMajors has Exception:%s", e.what());
				continue;
			}
			majorList.push_back(_major);
			_major = nullptr;
		}
		return true;
	}

	bool MajorService::changeMajor(int num, MajorBean::ptr newMajor) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		std::string sql;
		sql.append("update major set ");
		//if (oldMajor.get() == newMajor.get()) {
		//	m_error = "未修改数据";
		//	return false;
		//}
		if (newMajor->getNum() != -1) {
			sql.append("MNum=").append(std::to_string(newMajor->getNum()));
			sql.append(",");
		}
		if (!newMajor->getName().empty()) {
			sql.append("MName=\'").append(newMajor->getName()).append("\',");
		}
		if (newMajor->getDeparNum() != -1) {
			sql.append("DNum=").append(std::to_string(newMajor->getDeparNum()));
			sql.append(",");
		}
		if (!newMajor->getVersions().empty()) {
			sql.append("TableVersion=\'").append(newMajor->versionToJson().toStyledString().c_str());
			sql.append("\',");
		}
		if (sql.back() != ',') {
			m_error = "对象没有发生改变";
			return false;
		}
		sql.pop_back();
		sql.append(" where MNum=").append(std::to_string(num));
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}
}
