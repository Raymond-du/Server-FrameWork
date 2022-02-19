#ifndef __SEMAPHONE
#define __SEMAPHONE 

#include <semaphore.h>
#include <stdexcept>

namespace raymond{
        class Semaphore
        {
        private:
                sem_t m_sem;
        public:
                /**
                 * @brief 初始化信号量的大小
                 * 
                 * @param count  
                 */
                Semaphore(int count = 0);
                /**
                 * @brief  析构函数 停止sem
                 * 
                 */
                ~Semaphore();

                /**
                 * @brief 等待信号量的通知
                 * 
                 */
                void wait();

                /**
                 * @brief 添加 等待信号量
                 * 
                 * @param second 
                 * @return 返回时间是否超市
                 */
                bool wait(size_t second);
                
                /**
                 * @brief 通知信号量加一
                 * 
                 */
                void notify();
        };
        
}


#endif	// __SEMAPHONE
