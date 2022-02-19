#ifndef __CASLOCK_H
#define __CASLOCK_H 

#include <atomic>

namespace raymond{
        class CASLock{
        public:
                CASLock(){}
                ~CASLock(){}

                void lock(){
                        while(m_mutex.test_and_set()){}
                }

                void try_lock(){
                        m_mutex.test_and_set();
                }

                void unlock(){
                        m_mutex.clear();
                }

        private:
                std::atomic_flag m_mutex;
        };

}

#endif	// __CASLOCK_H
