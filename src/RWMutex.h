#ifndef __RWMUTEX_H
#define __RWMUTEX_H 

#include <thread>
#include <stdexcept>

namespace raymond{
        //使用了读锁 可以使用 c++ 11 std::mutex 进行加锁和释放锁
        class RLock{
        protected:
                //     读写锁
                pthread_rwlock_t* m_rlock;
        public:
                RLock(pthread_rwlock_t* rlock):m_rlock(rlock){}
                /**
                 * @brief 读加锁
                 * 
                 */
                void lock(){
                        if(pthread_rwlock_rdlock(m_rlock)){
                                throw std::logic_error("pthread_rwlock_rdlock error");
                        }
                }

                /**
                 * @brief 尝试获取读锁
                 * 
                 * @return  是否获得锁
                 */
                bool try_lock(){
                        if(pthread_rwlock_tryrdlock(m_rlock)){
                                if(errno == EBUSY){
                                        return false;
                                }else{
                                        throw std::logic_error("pthread_rwlock_tryrdlock error");
                                        return false;
                                }
                        }
                        return true;
                }
                /**
                 * @brief 解锁
                 * 
                 */
                void unlock(){
                        if(pthread_rwlock_unlock(m_rlock)){
                                throw std::logic_error("pthread_rwlock_unlock error");
                        }
                }
        };

        class WLock{
        protected:
                //     读写锁
                pthread_rwlock_t* m_wlock;
        public:
                WLock(pthread_rwlock_t * rwlock):m_wlock(rwlock){}
                /**
                 * @brief 写加锁
                 * 
                 */
                void lock(){
                        if(pthread_rwlock_wrlock(m_wlock)){
                                throw std::logic_error("pthread_rwlock_wrlock error");
                        }
                }
                /**
                 * @brief 尝试获取写锁
                 * 
                 * @return  是否获取到写锁
                 */
                bool try_lock(){
                        if(pthread_rwlock_trywrlock(m_wlock)){
                                if(errno == EBUSY){
                                        return false;
                                }else{
                                        throw std::logic_error("pthread_rwlock_trywrlock error");
                                        return false;
                                }
                        }
                        return true;
                }
                /**
                 * @brief 释放锁
                 * 
                 */
                void unlock(){
                        if(pthread_rwlock_unlock(m_wlock)){
                                throw std::logic_error("pthread_rwlock_unlock error");
                        }
                }
        };
        class RWMutex :public RLock, public WLock{
        private:
                pthread_rwlock_t m_rwlock;
        public:
                RWMutex():RLock(&m_rwlock),WLock(&m_rwlock){
                        if(pthread_rwlock_init(&m_rwlock,nullptr) != 0 ){
                                throw std::logic_error("pthread_rwlock_unlock error");
                        }
                }

                ~RWMutex(){
                        pthread_rwlock_destroy(&m_rwlock);
                }

                void rdlock(){
                        RLock::lock();
                }

                void wrlock(){
                        WLock::lock();
                }

                void tryrdlock(){
                        RLock::try_lock();
                }

                void trywrlock(){
                        WLock::lock();
                }

                void unlock(){
                        if(pthread_rwlock_unlock(m_wlock)){
                                throw std::logic_error("pthread_rwlock_unlock error");
                        }
                }

        };
        
}


#endif	// __RWMUTEX_H