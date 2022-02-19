#ifndef __ENTITY_MAJORBEAN_H
#define	__ENTITY_MAJORBEAN_H
#include "../singleton.h"
#include <string>
#include <memory>
#include <sstream>
#include <set>
#include <unordered_map>
#include "../json/json/json.h"

namespace raymond {
	class MajorBean {
	private:
		int m_num;
		std::string m_name;
		int m_departmentNum;
		std::set<std::string> m_versions;

	public:
		typedef std::shared_ptr<MajorBean> ptr;
		MajorBean() :	m_num(-1),
									m_name(""),
									m_departmentNum(-1) {}
		MajorBean(int num, const std::string& name, int d_Num) :
							m_num(num),
							m_name(name),
							m_departmentNum(d_Num) {}

		int getNum()		{ return m_num;}
		std::string getName() { return m_name;}
		int getDeparNum() { return m_departmentNum;}
		const std::set<std::string>& getVersions() { return m_versions;}

		bool addVersion(const std::string& version);
		bool delVersion(const std::string& version);
		Json::Value versionToJson();
		bool setVersion(const std::string& jsonStr);

		void setNum(int num) { m_num = num;}
		void setName(const std::string& name) { m_name = name;}
		void setDepartNum(int num) { m_departmentNum = num;}

		std::string toString() {
			std::stringstream ss;
			ss << "{MajorBean num:" << m_num << ",name:" << m_name << ",m_departmentNum:" << m_departmentNum << "}";
			return ss.str();
		}
	};

	//进行对major缓存
	//TODO: 在多线程的情况下会出现不同步的情况 之后需要加上锁
	//TODO: 初始化时从数据库获取major信息
	class MajorManager {
	friend class SingleTon<MajorManager>;

	private:
		std::unordered_map<int, MajorBean::ptr> m_MajorMap;
		std::string m_error;

	private:
		MajorManager() {
			init();
		}
		void init();
	public:
		static MajorManager* getInstance() {
			return SingleTon<MajorManager>::getInstance();
		}

		std::string getError() { return m_error;}
		
		const std::unordered_map<int, MajorBean::ptr>& getAll() {
			return m_MajorMap;
		}
		/**
		 * @brief getMajorId 获取major的ID
		 *
		 * @return 错误返回-1
		 */
		int getMajorId(const std::string& name);
		/**
		 * @brief find 根据编号查找major对象
		 *
		 */
		MajorBean::ptr find(int num);

		bool add(MajorBean::ptr major);

		bool del(int num);	

		bool changeMajor(int num, MajorBean::ptr major);
	};

}

#endif
