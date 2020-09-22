
#include "Semaphore.h"

namespace raymond{

        Semaphore::Semaphore(int count){
                if(sem_init(&m_sem,0,count)){
                        throw std::logic_error("sem_init error");
                }
        }

        Semaphore::~Semaphore(){
                sem_destroy(&m_sem);
        }

        void Semaphore::wait(){
                if(sem_wait(&m_sem)){
                        throw std::logic_error("sem_wait error");
                }
        }

        bool Semaphore::wait(size_t second){
                timespec ts;
                ts.tv_sec = second;
                if(sem_timedwait(&m_sem,&ts)){
                        if(errno == ETIMEDOUT){
                                return false;
                        }else{
                                throw std::logic_error("sem_timewait error");
                        }
                }
                return true;
        }

        void Semaphore::notify(){
                if(sem_post(&m_sem)){
                        throw std::logic_error("sem_post error");
                }
        }

}