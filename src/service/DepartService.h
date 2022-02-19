#ifndef __SERVICE_DEPARTSERVICE_H
#define __SERVICE_DEPARTSERVICE_H

#include "../entity/DepartmentBean.h"
#include "../sql/connection.h"
#include <memory>
#include <list>
namespace raymond {
	class DepartService {
	
	private:
		Connection* m_connection;
		std::string m_error;
	public:
		typedef std::shared_ptr<DepartService> ptr;

		DepartService();
		std::string getError() { return m_error;}
		DepartmentBean::ptr getDeparByNum(int num);
		bool addDepart(DepartmentBean::ptr depart);
		bool delDepartByNum(int num);
		bool getAllDeparts(std::list<DepartmentBean::ptr>& departList);
		bool changeDepart(int num, DepartmentBean::ptr newDepart);

	};
}

#endif
