#include "DepartmentBean.h"
#include "../service/DepartService.h"
#include <list>

namespace raymond {

	void DepartmentManager::init() {
		DepartService departService;
		std::list<DepartmentBean::ptr> departList;
		
		if (false == departService.getAllDeparts(departList)) {
			m_error = departService.getError();
			return;
		}
		for (auto& depart : departList) {
			m_DepartmentMap[depart->getNum()] = depart;
		}
		
	}
}
