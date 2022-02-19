#include "MajorBean.h"
#include "../service/majorService.h"
#include <memory>

namespace raymond {

	bool MajorBean::addVersion(const std::string& version) {
		auto it = m_versions.find(version);
		if (it != m_versions.end()) {
			return false;
		}
		m_versions.insert(version);
		return true;
	}

	bool MajorBean::delVersion(const std::string& version) {
		auto it = m_versions.find(version);
		if (it != m_versions.end()) {
			m_versions.erase(it);
			return true;
		}
		return false;
	}

	Json::Value MajorBean::versionToJson() {
		Json::Value value;
		for (auto& version : m_versions) {
			value.append(version);
		}
		return value;
	}

	bool MajorBean::setVersion(const std::string& jsonStr) {
		Json::CharReaderBuilder build;
		std::unique_ptr<Json::CharReader> reader(build.newCharReader());
		Json::Value value;
		JSONCPP_STRING err;
		reader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &value, &err);
		if (value.isArray()) {
			for (auto& vJson : value) {
				addVersion(vJson.asString());
			}
			return true;
		}
		return false;
	}

	void MajorManager::init() {
		MajorService majorService;
		std::list<MajorBean::ptr> majorList;
		if (false == majorService.getAllMajors(majorList)) {
			m_error = majorService.getError();
			return;
		}
		for (auto& major : majorList) {
			m_MajorMap[major->getNum()] = major;
		}
	}

	int MajorManager::getMajorId(const std::string& name) {
		for (auto& it : m_MajorMap) {
			if (it.second->getName().compare(name) == 0) {
				return it.first;
			}
		}
		return -1;
	}

	MajorBean::ptr MajorManager::find(int num) {
		auto it = m_MajorMap.find(num);
		if (it != m_MajorMap.end()) {
			return it->second;
		}
		m_error = "con't find this major";
		return nullptr;
	}
	
	bool MajorManager::add(MajorBean::ptr major) {
		auto it = m_MajorMap.find(major->getNum());
		if (it == m_MajorMap.end()) {
			//不存在则插入到数据库中
			MajorService majorService;
			if (false == majorService.addMajor(major)) {
				m_error = majorService.getError();
				return false;
			}
			m_MajorMap[major->getNum()] = major;
			return true;
		}
		m_error = "it is existed";
		return false;
	}

	bool MajorManager::del(int num) {
		auto it = m_MajorMap.find(num);
		if (it != m_MajorMap.end()) {

			//存在则在数据库中删除
			MajorService majorService;
			if (false == majorService.deleteMajorByNum(num)) {
				m_error = majorService.getError();
				return false;
			}
			m_MajorMap.erase(it);
			return true;
		}
		m_error = "can't find this major";
		return false;
	}

	bool MajorManager::changeMajor(int num, MajorBean::ptr major) {
		auto it = m_MajorMap.find(num);
		if (it != m_MajorMap.end()) {
			
			MajorService majorService;
			if (false == majorService.changeMajor(num, major)) {
				m_error = majorService.getError();
				return false;
			}
			if (major->getNum() != -1) {
				it->second->setNum(major->getNum());
			}
			if (!major->getName().empty()) {
				it->second->setName(major->getName());
			}
			if (major->getDeparNum() != -1) {
				it->second->setDepartNum(major->getDeparNum());
			}
		}
		m_error = "can't find this major";
		return false;
	}
}
