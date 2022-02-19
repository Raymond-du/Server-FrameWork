#ifndef __ENTITY_USERBEAN_H
#define __ENTITY_USERBEAN_H

#include "../entity/MajorBean.h"
#include <string>
#include <memory>
#include <sstream>

namespace raymond {
	class User {
	
	private:
		std::string m_id;
		int m_age;
		int m_privilege;
		std::string m_name;
		std::string m_passwd;
		std::string m_phone;
		std::string m_email;
		std::string m_jobTitle;
		int m_majorNum;
		std::string m_note;
	public:
		typedef std::shared_ptr<User> ptr;

		User() : m_id(""),
						 m_age(-1),
						 m_privilege(-1),
						 m_name(""),
						 m_passwd(""),
						 m_phone(""),
						 m_email(""),
						 m_jobTitle(""),
						 m_majorNum(-1),
						 m_note("") {}
		User(const std::string& id, 
				int age,
				int privilege,
				const std::string& name,
				const std::string& passwd,
				const std::string& phone,
				const std::string& email,
				const std::string& jobTitle,
				int major,
				const std::string& note) :
				m_id(id),
				m_age(age),
				m_privilege(privilege),
				m_name(name),
				m_passwd(passwd),
				m_phone(phone),
				m_email(email),
				m_jobTitle(jobTitle),
				m_majorNum(major),
				m_note(note) {
				}

		std::string getId()								{ return m_id;}
		int getAge()							{ return m_age;}
		int getPrivilege()				{ return m_privilege;}
		int getMajorNum()						{ return m_majorNum;}
		std::string getName()			{ return m_name;}
		std::string getPasswd()		{ return m_passwd;}
		std::string getPhone()		{ return m_phone;}
		std::string getEmail()		{ return m_email;}
		std::string getJobTitle() {return m_jobTitle;}
		std::string getNote()			{ return m_note;}
		std::string getMajorName() {
			auto majorManager = MajorManager::getInstance();
			auto major = majorManager->find(m_majorNum);
			if (major != nullptr) {
				return major->getName();
			}
			return "未知";
		}

		void setId(const std::string& id)			{ m_id = id;}
		void setAge(int age)		{ m_age = age;} 
		void setPrivilege(int privilege) { m_privilege = privilege;}
		void setMajorNum(int majorNum)			{ m_majorNum = majorNum;}
		void setName		(const std::string& name)				{ m_name = name;}
		void setPasswd	(const std::string& passwd)			{ m_passwd = passwd;}
		void setPhone		(const std::string& phone)			{ m_phone = phone;}
		void setEmail		(const std::string& email)			{ m_email = email;}
		void setJobTitle(const std::string& jobTitle)		{ m_jobTitle = jobTitle;}
		void setNote		(const std::string& note)				{ m_note = note;}
		void setMajorName (const std::string& name) {
			auto majorManager = MajorManager::getInstance();
			auto majorid = majorManager->getMajorId(name);
			if (majorid != -1) {
				m_majorNum = majorid;
			}
			m_majorNum = -1;
		}

		std::string toString() {
			std::stringstream ss;
			ss << "{id: " << m_id << ",age:" << m_age << ",privilege:" << m_privilege 
					<< ",name: " << m_name << ",passwd:" << m_passwd << ",phone: " << m_phone 
					<< ",email: " << m_email << ",jobTitle:" << m_jobTitle 
					<< ",major:" << m_majorNum << ",note:" << m_note << "}";
			return ss.str();
		}
	};
}

#endif
