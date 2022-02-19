#ifndef __ENTITY_DEPARTMENTBEAN_H
#define	__ENTITY_DEPARTMENTBEAN_H
#include "../singleton.h"
#include <string>
#include <memory>
#include <sstream>
#include <unordered_map>

namespace raymond {
	class DepartmentBean {
	private:
		int m_num;
		std::string m_name;

	public:
		typedef std::shared_ptr<DepartmentBean> ptr;
		DepartmentBean() :	m_num(-1),
									m_name("") {}
		DepartmentBean(int num, const std::string& name) :
							m_num(num),
							m_name(name) {}

		int getNum()		{ return m_num;}
		std::string getName() { return m_name;}

		void setNum(int num) { m_num = num;}
		void setName(const std::string& name) { m_name = name;}

		std::string toString() {
			std::stringstream ss;
			ss << "{DepartmentBean num:" << m_num << ",name:" << m_name << "}";
			return ss.str();
		}
	};

	//进行对major缓存
	//TODO: 在多线程的情况下会出现不同步的情况 之后需要加上锁
	class DepartmentManager {
	
	friend class SingleTon<DepartmentManager>;

	private:
		std::unordered_map<int, DepartmentBean::ptr> m_DepartmentMap;
		std::string m_error;

	private:
		DepartmentManager() {
			init();
		}
		void init();

	public:
		static DepartmentManager* getInstance() {
			return SingleTon<DepartmentManager>::getInstance();
		}

		const std::unordered_map<int, DepartmentBean::ptr>& getAll() {
			return m_DepartmentMap;
		}

		std::string getError() { return m_error;}
		/**
		 * @brief find 根据编号查找major对象
		 *
		 */
		DepartmentBean::ptr find(int num) {
			auto it = m_DepartmentMap.find(num);
			if (it != m_DepartmentMap.end()) {
				return it->second;
			}
			return nullptr;
		}

		bool add(DepartmentBean::ptr major) {
			auto it = m_DepartmentMap.find(major->getNum());
			if (it != m_DepartmentMap.end()) {
				m_DepartmentMap[major->getNum()] = major;
				return true;
			}
			return false;
		}

		bool del(int num) {
			auto it = m_DepartmentMap.find(num);
			if (it != m_DepartmentMap.end()) {
				m_DepartmentMap.erase(it);
			}
			return true;
		}
	
	};

}

#endif
