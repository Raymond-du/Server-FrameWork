#ifndef __THREAD_H
#define __THREAD_H

#include <string>
#include <thread>
#include <unistd.h>
#include <functional>
#include <future>
#include <utility>
#include <iostream>
#include "Logger.h"
#include "Semaphore.h"

namespace raymond{
        class Thread;

        static thread_local std::string t_threadName = "NULL";
        class Thread{
        public:
                typedef std::shared_ptr<Thread> ptr;

                template<class F, class... Args>
                explicit Thread(const std::string & name,F && func,Args&&... args){
                        using result_type = typename std::result_of<F(Args...)>::type;
                        auto t =std::make_shared< std::packaged_task<result_type()> >(
                                                std::bind(std::forward<F>(func),std::forward<Args>(args)...));
                        m_thread = std::thread([t,name,this]{
                                t_threadName = name;
                                this->m_id = getCurId();
                                this->m_sem.notify();
                                (*t)();
                        });
                        //等待启动线程初始化完对象
                        m_sem.wait();
                }
                ~Thread();
                /**
                 * @brief 获取当前线程的name
                 * 
                 * @return  线程名称
                 */
                static const std::string & getCurName(){return t_threadName;}
                /**
                 * @brief 设置当前线程的name
                 * 
                 * @param name 
                 */
                static void setCurName(const std::string & name){
                        t_threadName = name;
                }
                /**
                 * @brief 获取当前对象的线程名称
                 * 
                 * @return  线程名称
                 */
                const std::string & getName(){return m_name;}
                /**
                 * @brief  获取线程是否joinable
                 * 
                 * @return  bool
                 */
                bool joinable(){return m_thread.joinable();}
                /**
                 * @brief  分离线程对象
                 * 
                 */
                void detach(){m_thread.detach();}
                /**
                 * @brief  等待线程执行完成
                 * 
                 */
                void join();
                /**
                 * @brief 获取当前对象的线程id
                 * 
                 * @return     线程id
                 */
                int32_t getId(){return m_id;}
                /**
                 * @brief 获取当前的线程id
                 * 
                 * @return 
                 */
                static int32_t getCurId(){
                        return syscall(186);
                }

        private:
                //将cope复制禁用
                Thread operator=(const Thread &) = delete;
        
        private:
                std::string m_name = "NULL";
                int32_t m_id = -1;;
                std::thread m_thread;
                Semaphore m_sem;  //保证该对象的初始化完成
        };

}

#endif	// __THREAD_H
