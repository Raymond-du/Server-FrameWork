#include "hook.h"
#include "Logger.h"
#include "fiber.h"
#include "iomanager.h"
#include "schedule.h"
#include "timer1.h"
#include <dlfcn.h>

namespace raymond{
	static raymond::Logger::ptr g_logger = RAYMOND_LOG_BYNAME("system");
	static thread_local bool t_hook_enable = false;

	bool is_hook_enable(){
		return t_hook_enable;
	}

	void set_hook_enable(bool flag){
		t_hook_enable = flag;
	}

#define HOOK_FUN(xx) \
	xx(sleep) 

	void hook_init(){
		static bool is_inited = false;
		if(is_inited){
			return;
		}
		//将原始的函数保存到name+_f的函数中
#define xx(name) name ## _f = (name ## _func)dlsym(RTLD_NEXT,#name);
		HOOK_FUN(xx);
#undef xx
	}

	struct _InitHook{
		_InitHook(){
			hook_init();
		}
	};
	
	static _InitHook s_hook_init;
}

extern "C"{
#define xx(name) name ## _func name ## _f  = nullptr ;
	HOOK_FUN(xx);
#undef xx

	unsigned int sleep(unsigned int seconds){
		if(!raymond::is_hook_enable()){
			return sleep_f(seconds);
		}

		auto fiber = raymond::Fiber::getCurFiber();
		// raymond::FiberSchedule::ptr schedule = raymond::FiberSchedule::getScheduler();
		raymond::IOManager* ioManager = raymond::IOManager::getIOManager();
		//使用定时器
		ioManager->addTimer(seconds * 1000,[ioManager,fiber](){
			fiber->setStatus(raymond::Fiber::READY);
			ioManager->schedule(fiber);
		},0);

		fiber->swapToHold();
		RAYMOND_LOG_FMT_DEBUG(raymond::g_logger,"continue fiber");
		return 0;
	}
}
