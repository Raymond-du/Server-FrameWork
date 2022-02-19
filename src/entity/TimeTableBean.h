#ifndef __ENTITY_TIMETABLEBEAN_H
#define __ENTITY_TIMETABLEBEAN_H

#include <list>
#include <memory>
#include <string>
namespace raymond {
	class CourseBean {
		
	private:
		int m_tNum = -1;
		std::string m_tName;
		std::string m_engName;
		int m_tCridet = -1;
		std::string m_tTrainMode;
		std::string m_tProperty;
		int m_tEnabled = -1;
		std::string m_tSubmitter;
		std::string m_tAuditor;
		int m_tPeriod = -1; //学期
		int m_tOfferSemester = -1; //执行学期
		int m_tTeachWeak = -1;
		std::string m_testMode;
		std::string m_tOfferedDName;
		int m_isMajor = -1;
		int m_isKernel = -1;
		std::string m_tNote;
		int m_tOprtPermission = -1;//需要的执行权限
		int m_tIsInClass = -1; //是否是课堂中上课
		std::string m_tCourseType;

	public:
		typedef std::shared_ptr<CourseBean> ptr;

		int getTNum() { return m_tNum;}
		std::string getTName() { return m_tName;}
		std::string getEngName() { return m_engName;}
		int getTCridet() { return m_tCridet;}
		std::string getTTrainMode() { return m_tTrainMode;}
		std::string getTProperty() { return m_tProperty;}
		int getIsEnabled() { return m_tEnabled;}
		std::string getTSubmitter() { return m_tSubmitter;}
		std::string getTAuditor() { return m_tAuditor;}
		int getTPeriod() { return m_tPeriod;}
		int getTOfferSemester() { return m_tOfferSemester;}
		int getTTeachWeak() { return m_tTeachWeak;}
		std::string getTestMode() { return m_testMode;}
		std::string getTOfferedDName() { return m_tOfferedDName;}
		int getIsMajor() { return m_isMajor;}
		int getIsKernel() { return m_isKernel;}
		std::string getTNote() { return m_tNote;}
		int getOprtPermission() { return m_tOprtPermission;}
		int getTIsInClass() { return m_tIsInClass;}
		std::string getTCourseType() { return m_tCourseType;}

		void setTNum(int num) { m_tNum = num;}
		void setTName(const std::string& name) { m_tName = name;}
		void setEngName(const std::string& name) { m_engName = name;}
		void setTCridet(int cridet) { m_tCridet = cridet;}
		void setTTrainMode(const std::string& mode) { m_tTrainMode = mode;}
		void setTProperty(const std::string& property) { m_tProperty = property;}
		void setIsEnabled(int v) { m_tEnabled = v;}
		void setTSubmitter(const std::string& uid) { m_tSubmitter = uid;}
		void setTAuditor(const std::string& uid) { m_tAuditor = uid;}
		void setTPeriod(int period) { m_tPeriod = period;}
		void setTOfferSemester(int offerSemester) { m_tOfferSemester = offerSemester;}
		void setTTeachWeak(int n) { m_tTeachWeak = n;}
		void setTestMode(const std::string& mode) { m_testMode = mode;}
		void setTOfferedDName(const std::string& dName) { m_tOfferedDName = dName;}
		void setIsMajor(int v) { m_isMajor = v;}
		void setIsKernel(int v) { m_isKernel = v;}
		void setTNote(const std::string& note) { m_tNote = note;}
		void setOprtPermission(int per) { m_tOprtPermission = per;}
		void setIsInClass(int v) { m_tIsInClass = v;}
		void setTCourseType(const std::string& type) { m_tCourseType = type;}
	};

	class TimeTable {
		
	private:
		std::list<CourseBean::ptr> m_courses;

	public:
		typedef std::shared_ptr<TimeTable> ptr;
		const std::list<CourseBean::ptr>& getCourses() { return m_courses;}
		void addCourse(CourseBean::ptr course) { m_courses.push_back(course);}
	};
}

#endif
