
#include "TimeTableService.h"
#include <cstdio>
#include "../Logger.h"
#include "../entity/TimeTableBean.h"
#include "../util/sqlutil.h"
#include <string>

namespace raymond {
	static auto g_logger = RAYMOND_LOG_BYNAME("system");

	TimeTableService::TimeTableService() {
		m_connection = getConnection();
	}

	TimeTable::ptr TimeTableService::getTimeTable(const std::string& name) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return nullptr;
		}
		auto statement = m_connection->createStatement();
		std::string sql = "select TNum,TName,TEngName,TCredit,TTrainMode,TProperty,TEnabled,TSubmitter,TAuditor,TPeriod,TOfferSemester,TTeachWeeks,TTestMode,TOfferedDName,TIsMajor,TIsKernel,TNote,TOperatePermission,TIsInClass,TCourseType from " + name;
		if (false == statement->query(sql)) {
			return nullptr;
		}
		auto resultset = statement->getResultSet();
		TimeTable::ptr table(new TimeTable());
		CourseBean::ptr course;
		while (resultset->nextResult()) {
			course.reset(new CourseBean());
			course->setTNum(resultset->getInt(0));
			course->setTName(resultset->getString(1));
			course->setEngName(resultset->getString(2));
			course->setTCridet(resultset->getInt(3));
			course->setTTrainMode(resultset->getString(4));
			course->setTProperty(resultset->getString(5));
			course->setIsEnabled(resultset->getBool(6));
			course->setTSubmitter(resultset->getString(7));
			course->setTAuditor(resultset->getString(8));
			course->setTPeriod(resultset->getInt(9));
			course->setTOfferSemester(resultset->getInt(10));
			course->setTTeachWeak(resultset->getInt(11));
			course->setTestMode(resultset->getString(12));
			course->setTOfferedDName(resultset->getString(13));
			course->setIsMajor(resultset->getBool(14));
			course->setIsKernel(resultset->getBool(15));
			course->setTNote(resultset->getString(16));
			course->setOprtPermission(resultset->getInt(17));
			course->setIsInClass(resultset->getInt(18));
			course->setTCourseType(resultset->getString(19));
			table->addCourse(course);
			course = nullptr;
		}
		return table;
	}
	
	bool TimeTableService::createTimeTable(const std::string& name) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		std::string sql = "create table " + name;
		sql.append("(TNum int(11),TName varchar(32),TEngName char(64),TCredit int(11),TTrainMode varchar(16),TProperty varchar(16),TEnabled tinyint(4),TSubmitter varchar(16),TAuditor varchar(16),TPeriod int(11),TOfferSemester int(11),TTeachWeeks int(11),TTestMode varchar(16),TOfferedDName varchar(32),TIsMajor tinyint(4),TIsKernel tinyint(4),TNote varchar(64),TOperatePermission int(11),TIsInClass int(2),TCourseType varchar(16),PRIMARY KEY (TNum)) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool TimeTableService::delTimeTable(const std::string& name) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		std::string sql = "drop table " + name;
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool TimeTableService::changeCourse(const std::string& name, int num, CourseBean::ptr course) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		
		std::string sql;
		sql.append("update ").append(name).append(" set ");
		if (course->getTNum() != -1) {
			sql.append("TNum=").append(std::to_string(course->getTNum()));
			sql.append(",");
		}
		if (!course->getTName().empty()) {
			sql.append("TName=\'").append(course->getTName());
			sql.append("\',");
		}
		if (!course->getEngName().empty()) {
			sql.append("TEngName=\'").append(course->getEngName());
			sql.append("\',");
		}
		if (course->getTCridet() != -1) {
			sql.append("TCredit=").append(std::to_string(course->getTCridet()));
			sql.append(",");
		}
		if (!course->getTTrainMode().empty()) {
			sql.append("TTrainMode=\'").append(course->getTTrainMode());
			sql.append("\',");
		}
		if (!course->getTProperty().empty()) {
			sql.append("TProperty=\'").append(course->getTProperty());
			sql.append("\',");
		}
		if (course->getIsEnabled() != -1) {
			sql.append("TEnabled=").append(std::to_string(course->getIsEnabled()));
			sql.append(",");
		}
		if (!course->getTSubmitter().empty()) {
			sql.append("TSubmitter=\'").append(course->getTSubmitter());
			sql.append("\',");
		}
		if (!course->getTAuditor().empty()) {
			sql.append("TAuditor=\'").append(course->getTAuditor());
			sql.append("\',");
		}
		if (course->getTPeriod() != -1) {
			sql.append("TPeriod=").append(std::to_string(course->getTPeriod()));
			sql.append(",");
		}
		if (course->getTOfferSemester() != -1) {
			sql.append("TOfferSemester=").append(std::to_string(course->getTOfferSemester()));
			sql.append(",");
		}
		if (course->getTTeachWeak() != -1) {
			sql.append("TTeachWeaks=").append(std::to_string(course->getTTeachWeak()));
			sql.append(",");
		}
		if (!course->getTestMode().empty()) {
			sql.append("TTestMode=\'").append(course->getTestMode());
			sql.append("\',");
		}
		if (!course->getTOfferedDName().empty()) {
			sql.append("TOfferedDName=\'").append(course->getTOfferedDName());
			sql.append("\',");
		}
		if (course->getIsMajor() != -1) {
			sql.append("TIsMajor=").append(std::to_string(course->getIsMajor()));
			sql.append(",");
		}
		if (course->getIsKernel() != -1) {
			sql.append("TIsKernel=").append(std::to_string(course->getIsKernel()));
			sql.append(",");
		}
		if (!course->getTNote().empty()) {
			sql.append("TNote=\'").append(course->getTNote());
			sql.append("\',");
		}
		if (course->getOprtPermission() != -1) {
			sql.append("TOperatePermission=").append(std::to_string(course->getOprtPermission()));
			sql.append(",");
		}
		if (course->getTIsInClass() != -1) {
			sql.append("TIsInClass=").append(std::to_string(course->getTIsInClass()));
			sql.append(",");
		}
		if (!course->getTCourseType().empty()) {
			sql.append("TCourseType=\'").append(course->getTCourseType());
			sql.append("\',");
		}
		if (sql.back() != ',') {
			m_error = "该对象没有改变";
			return false;
		}
		sql.pop_back();
		sql.append(" where TNum=").append(std::to_string(num));
		if (false == statement->query(sql)) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;

	}

	bool TimeTableService::addCourse(const std::string& name, CourseBean::ptr course) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();

		char sql[1024 * 100];
		snprintf(sql, 1024 * 100, "insert into %s(TNum,TName,TEngName,TCredit,TTrainMode,TProperty,TEnabled,TSubmitter,TAuditor,TPeriod,TOfferSemester,TTeachWeeks,TTestMode,TOfferedDName,TIsMajor,TIsKernel,TNote,TOperatePermission,TIsInClass,TCourseType) value(%d,\'%s\',\'%s\',%d,\'%s\',\'%s\',%d,\'%s\',\'%s\',%d,%d,%d,\'%s\',\'%s\',%d,%d,\'%s\',%d,%d,\'%s\')", 
				name.c_str(),
				course->getTNum(),
				course->getTName().c_str(),
				course->getEngName().c_str(),
				course->getTCridet(),
				course->getTTrainMode().c_str(),
				course->getTProperty().c_str(),
				course->getIsEnabled(),
				course->getTSubmitter().c_str(),
				course->getTAuditor().c_str(),
				course->getTPeriod(),
				course->getTOfferSemester(),
				course->getTTeachWeak(),
				course->getTestMode().c_str(),
				course->getTOfferedDName().c_str(),
				course->getIsMajor(),
				course->getIsKernel(),
				course->getTNote().c_str(),
				course->getOprtPermission(),
				course->getTIsInClass(),
				course->getTCourseType().c_str());
		if (false == statement->query(std::string(sql))) {
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}

	bool TimeTableService::delCourse(const std::string& name, int tnum) {
		if (m_connection == nullptr) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "与数据库未建立连接");
			return false;
		}
		auto statement = m_connection->createStatement();
		std::string sql = "delete from " + name + " where TNum=" + std::to_string(tnum);
		if (false == statement->query(sql)) {
			RAYMOND_LOG_FMT_ERROR(g_logger, "exec fatal sql:%s", sql.c_str());
			m_error = statement->getError();
			return false;
		}
		m_connection->commit();
		return true;
	}
}
