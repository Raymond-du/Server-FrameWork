#ifndef __APPLICATION_H
#define __APPLICATION_H

#include "iomanager.h"
#include "servlet/servlet.h"

namespace raymond {
	class Application {
		
	private:
		char** m_argv = nullptr;
		raymond::IOManager::ptr m_IOManager;


	public:
		Application();
		Application(int argc, char** argv);

		void init();
		int run();
	private:
		void work_func();

	};

}

#endif
