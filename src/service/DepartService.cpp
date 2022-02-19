#include "DepartService.h"
#include "../util/sqlutil.h"
#include "../Logger.h"

namespace raymond {

	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	DepartService::DepartService() {
		m_connection = getConnection();
	}

	bool DepartService::addDepart(DepartmentBean::ptr depart) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		char sql[256];
		snprintf(sql, 256, "insert into department(DNum, DName) value(%d,%s)",depart->getNum(), depart->getName().c_str());
		if (false == statement->query(std::string(sql))) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}
	
	bool DepartService::delDepartByNum(int num) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		std::string sql = "delete from department where DNum=" + std::to_string(num);
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool DepartService::getAllDeparts(std::list<DepartmentBean::ptr>& departList) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		if (false == statement->query("select DNum,DName from department")) {
			m_error = statement->getError();
			RAYMOND_LOG_FMT_ERROR(g_logger, "执行sql语句出现");
			return false;
		}
		auto resultset = statement->getResultSet();
		DepartmentBean::ptr depart = nullptr;
		while (resultset->nextResult()) {
			try {
				depart.reset(new DepartmentBean());
				depart->setNum(resultset->getInt(0));
				depart->setName(resultset->getString(1));
			} catch (std::exception& e) {
				RAYMOND_LOG_FMT_ERROR(g_logger, "getAllMajors has Exception:%s", e.what());
				continue;
			}
			departList.push_back(depart);
			depart = nullptr;
		}
		return true;
	
	}

	bool DepartService::changeDepart(int num, DepartmentBean::ptr newDepart) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		std::string sql;
		sql.append("update department set ");
		//if (oldMajor.get() == newMajor.get()) {
		//	m_error = "未修改数据";
		//	return false;
		//}
		if (newDepart->getNum() != -1) {
			sql.append("DNum=").append(std::to_string(newDepart->getNum()));
			sql.append(",");
		}
		if (!newDepart->getName().empty()) {
			sql.append("DName=\'").append(newDepart->getName()).append("\',");
		}
		if (sql.back() != ',') {
			m_error = "对象没有发生改变";
			return false;
		}
		sql.pop_back();
		sql.append(" where DNum=").append(std::to_string(num));
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	
	}
}
