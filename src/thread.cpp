
#include "thread.h"

namespace raymond{

        Thread::~Thread(){
               if(m_thread.joinable())
                        m_thread.detach();
                

        }

        void Thread::join(){
                m_thread.join();
        }


}