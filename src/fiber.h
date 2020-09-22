
#ifndef __FIBER_H
#define __FIBER_H

#include <cstddef>
#include <memory>
#include <functional>
#include <sys/ucontext.h>
#include <type_traits>
#include <ucontext.h>
#include <future>
#include <utility>

namespace raymond{

        class Fiber : public std::enable_shared_from_this<Fiber>{
        public:
                typedef std::shared_ptr<Fiber> ptr;

                //协程的状态
                enum Status{
                        INIT,//初始化状态
                        HOLD,//暂停状态
                        EXEC,//执行状态
                        TERM,//结束状态
                        READY,//可执行状态
			EXCEPT//异常状态
                };
        private:
		//私有.进行主协程的构造
                Fiber();

        public:
		/**
		 * @brief Fiber 创建一个协程
		 *
		 * @param func	需要执行的函数
		 * @param stacksize 设置栈的大小
		 * @param bySche    是否被调度器统一调度 默认不服从调度
		 */
		Fiber(std::function<void()> func,size_t stacksize = 0,bool bySche = false);
		//协程析构 释放栈,协程数减一
		~Fiber();

		/**
		 * @brief  重置协程的函数,状态 ,
		 *      INIT ,term  协程执行结束保留栈空间,执行下一个函数
		 * @param func  函数
		 */
		void reset(std::function<void()> func);
		/**
		 * @brief  切换到当前的协程执行
		 * 
		 */
		void swapIn();
		/**
		 * @brief  当前协程切换到后台
		 * 
		 */
		void swapOut();

		/**
		 * @brief swapToHold 将当前的协程切换成hold
		 */
		void swapToHold();

		/**
		 * @brief getStatus 获取该fiber的状态
		 *  
		 * @return 状态
		 */
		Status getStatus();

		/**
		 * @brief 获取fiberid
		 * 
		 * @return id:
		 */
		uint32_t getFiberId();

		/**
		 * @brief setStatus 设置该fiber的状态
		 *
		 * @param status 状态
		 */
		void setStatus(Status status);

		std::function<void()> getFunc();

        public:
		/**
		 * @brief 获取当前的协程
		 * 
		 * @return  协程的智能指针
		 */
		static Fiber::ptr getCurFiber();

		/**
		 * @brief 协程切换到后台,   设置为ready状态
		 * 
		 */
		static void yeildToReady();
		/**
		 * @brief 协程切换到后台 ,设置为HOLD状态
		 * 
		 */
		static void yeildToHold();
		/**
		 * @brief 获取全部的协程的数量
		 * 
		 * @return 
		 */
		static uint32_t getFibersNum();

		// static void setCurFiber(Fiber::ptr fiber);
		//协程执行函数 执行结束返回到主协程或者调度函数
		static void  mainFunc();

        private:
		uint32_t m_id = 0;
		uint32_t m_stackSize = 0;
		Status m_status = INIT;
		// 结束或者swap到改协程里
		Fiber::ptr m_mainFiber;
		void * m_stack = nullptr;

		std::function<void()> m_func;
		/**
		 * @brief 是否接受调度器的调度
		 */
		bool m_bySche = false;
			ucontext_t m_context;
        };

}

#endif	// __FIBER_H
