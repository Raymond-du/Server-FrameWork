#ifndef __SQL_RESULT_H
#define __SQL_RESULT_H

#include <cstring>
#include <sstream>
#include <string>
#include <memory>
#include "../byte.h"

namespace raymond {

	class Date {
	private:
		uint16_t m_year = 0;
		uint8_t m_month = 0;
		uint8_t m_day = 0;
	public:
		typedef std::shared_ptr<Date> ptr;

		Date(int year = 0, int month = 0, int day = 0)
				:m_year(year), m_month(month), m_day(day) {}

		Date(char* str) {
			char* tmp = nullptr;
			char* pch = ::strtok_r(str, "-", &tmp);
			m_year = (pch == nullptr ? 0 : atoi(pch));
			pch = strtok_r(nullptr, "-", &tmp);
			m_month = (pch == nullptr ? 0 : atoi(pch));
			pch = strtok_r(nullptr, "-", &tmp);
			m_day = (pch == nullptr ? 0 : atoi(pch));
		}

		int getYear() { return m_year;}
		int getmonth() { return m_month;}
		int getDay() { return m_day;}
		void setYear(int year) { m_year = year;}
		void setMonth(int month) { m_month = month;}
		void setDay(int day) { m_day = day;}

		std::string toString() {
			std::stringstream ss;
			ss << (int)m_year << "-" << (int)m_month << "-" << (int)m_day;
			return ss.str();
		}
	};

	class Time {
	private:
		uint8_t m_hour = 0;
		uint8_t m_minute = 0;
		uint8_t m_second = 0;
	public:
		typedef std::shared_ptr<Time> ptr;
		Time(int hour = 0, int minute = 0, int second = 0)
				:m_hour(hour), m_minute(minute), m_second(second) {}

		Time(char* str) {
			char* tmp = nullptr;
			char* pch = strtok_r(str, ":", &tmp);
			m_hour = (pch == nullptr ? 0 : atoi(pch));
			pch = strtok_r(nullptr, ":", &tmp);
			m_minute = (pch == nullptr ? 0 : atoi(pch));
			pch = strtok_r(nullptr, ":", &tmp);
			m_second = (pch == nullptr ? 0 : atoi(pch));
		}

		int getHour() { return m_hour;}
		int getMinute() { return m_minute;}
		int getSecond() { return m_second;}
		void setHour(int hour) { m_hour = hour;}
		void setMinute(int minute) { m_minute = minute;}
		void setSecond(int second) { m_second = second;}

		std::string toString() {
			std::stringstream ss;
			ss << (int)m_hour << ":" << (int)m_minute << ":" << (int)m_second;
			return ss.str();
		}
	};

	class DateTime {
	private:
		Date m_date;
		Time m_time;
	public:
		typedef std::shared_ptr<DateTime> ptr;

		DateTime(int year = 0, int month = 0, int day = 0
						,int hour = 0, int minute = 0, int second = 0)
						:m_date(year, month, day)
						,m_time(hour, minute, second) {}

		DateTime(char* str) {
			char* tmp = nullptr;
			char* pch = ::strtok_r(str, " ", &tmp);
			m_date = Date(pch);
			pch = ::strtok_r(nullptr, " ", &tmp);
			m_time = Time(pch);
		}

		int getYear() { return m_date.getYear();}
		int getmonth() { return m_date.getmonth();}
		int getDay() { return m_date.getDay();}
		void setYear(int year) { m_date.setYear(year);}
		void setMonth(int month) { m_date.setMonth(month);}
		void setDay(int day) { m_date.setDay(day);}
		int getHour() { return m_time.getHour();}
		int getMinute() { return m_time.getMinute();}
		int getSecond() { return m_time.getSecond();}
		void setHour(int hour) { m_time.setHour(hour);}
		void setMinute(int minute) { m_time.setMinute(minute);}
		void setSecond(int second) { m_time.setSecond(second);}

		std::string toString() {
			return m_date.toString() + " " + m_time.toString();
		}
	};

	class ResultSet {
	
	public:
		typedef std::shared_ptr<ResultSet> ptr;

		ResultSet() {}
		virtual bool getBool(int columnindex) = 0;
		virtual bool getBool(const std::string& columnlable) = 0;
		virtual int getInt(int columnindex) = 0;
		virtual int getInt(const std::string& columnlable) = 0;
		virtual float getFloat(int columnindex) = 0;
		virtual float getFloat(const std::string& columnlable) = 0;
		virtual long getLong(int columnindex) = 0;
		virtual long getLong(const std::string& columnlable) = 0;
		virtual double getDouble(int columnindex) = 0;
		virtual double getDouble(const std::string& columnlable) = 0;
		virtual std::string getString(int columnindex) = 0;
		virtual std::string getString(const std::string& columnlable) = 0;
		virtual Byte::ptr getBytes(int columnIndex) = 0;
		virtual Byte::ptr getBytes(const std::string& columnlable) = 0;
		virtual char* getRawData(int columnIndex, int* length) = 0;
		virtual char* getRawData(const std::string& columnLable, int* length) = 0;
		virtual Time::ptr getTime(int columnIndex) = 0;
		virtual Time::ptr getTime(const std::string& columnLable) = 0;
		virtual Date::ptr getDate(int columnIndex) = 0;
		virtual Date::ptr getDate(const std::string& columnLable) = 0;
		virtual DateTime::ptr getDateTime(int columnIndex) = 0;
		virtual DateTime::ptr getDateTime(const std::string& columnLable) = 0;

		virtual bool nextResult() = 0;
	};
}

#endif
