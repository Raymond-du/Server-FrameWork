#ifndef __SERVICE_TIMETABLESERVICE_H
#define __SERVICE_TIMETABLESERVICE_H

#include <memory>
#include "../sql/connection.h"
#include "../entity/TimeTableBean.h"
namespace raymond {

	class TimeTableService {
	
	private:
		Connection* m_connection;
		std::string m_error;

	public:
		typedef std::shared_ptr<TimeTableService> ptr;
		TimeTableService();

		std::string getError() { return m_error;}
		// 54_2020
		TimeTable::ptr getTimeTable(const std::string& name);
		bool createTimeTable(const std::string& name);
		bool delTimeTable(const std::string& name);
		bool changeCourse(const std::string& tablename, int num, CourseBean::ptr course);
		
		bool addCourse(const std::string& name, CourseBean::ptr course);
		bool delCourse(const std::string& name, int tnum);
	};
}

#endif
