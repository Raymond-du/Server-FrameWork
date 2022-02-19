#ifndef __SERVICE_MAJORSERVICE_H
#define __SERVICE_MAJORSERVICE_H

#include "../entity/MajorBean.h"
#include "../sql/connection.h"
#include <memory>
#include <list>

namespace raymond {
class MajorService {
	
private:
	Connection* m_connection;
	std::string m_error;

public:
	typedef std::shared_ptr<MajorService> ptr;

	MajorService();
	std::string getError() { return m_error;}
	MajorBean::ptr getMajorByNum(int num);
	bool addMajor(MajorBean::ptr major);
	bool deleteMajorByNum(int num);
	bool getAllMajors(std::list<MajorBean::ptr>& majorList);
	bool changeMajor(int num, MajorBean::ptr newMajor);
};
}

#endif
