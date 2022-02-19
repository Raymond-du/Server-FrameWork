#ifndef NLOCKQUEUE_HPP
#define NLOCKQUEUE_HPP

#include <atomic>
#include <vector>
#include <cstring>


template<class T>    
class NLockQueue 
{
public:
    NLockQueue(size_t baseSize = 1000) {
        T* data = new T[baseSize];
        m_nodes.push_back(data);
        m_baseSize = baseSize;
        m_capacity = m_baseSize;
        
        m_useSize = 0;
        m_readPos = 0;
        m_writePos = 0;
        m_expanding = false;
        m_poping = 0;
        m_pushing = 0;
        m_useCount = 0;
    }   
    
    ~NLockQueue() {
        //析构可能存在问题
        m_expanding.store(true);
        m_poping.store(true);
        m_pushing.store(true);
        for (auto& it : m_nodes) {
            delete[] it;
        }
    }
    
    bool try_pop(T& value) {
        if (m_useSize.fetch_sub(1) <= 0) {
            m_useSize.fetch_add(1);
            return false;
        }
        while (m_expanding.load(std::memory_order_relaxed) == true) {}
        m_poping.fetch_add(1);
        uint32_t pos = m_readPos.fetch_add(1) % m_capacity;
        uint32_t n = pos / m_baseSize;
        uint32_t npos = pos % m_baseSize; 
        value = m_nodes[n][npos];
        m_poping.fetch_sub(1);
        m_useCount.fetch_sub(1);
        return true;
    }
    
    void push(const T& value) {
        if (m_useCount.fetch_add(1) >= m_capacity) {
            bool ing = false;
            if (m_expanding.compare_exchange_strong(ing, true)) {
                expanding();
            }
        }
        while (m_expanding.load(std::memory_order_relaxed) == true) {}
        m_pushing.fetch_add(1);
        uint32_t pos = m_writePos.fetch_add(1) % m_capacity; 
        uint32_t n = pos / m_baseSize;
        uint32_t npos = pos % m_baseSize;
        m_nodes[n][npos] = value;
        m_useSize.fetch_add(1); //先添加进去 ysesize再加1
        m_pushing.fetch_sub(1);
    }
    
    bool empty() {
        return m_useSize == 0;
    }
    
private:
    void expanding() {
        //保证没有线程进行pop和push
        while (m_poping.load(std::memory_order_relaxed) != 0 || m_pushing.load(std::memory_order_relaxed) != 0) {}

        //存在回环问题  (不是按顺序进行)  pop的位置不变  push的位置改为移动后的位置
        uint32_t wPos = m_writePos.load(std::memory_order_relaxed) % m_capacity;
        uint32_t rPos = m_readPos.load(std::memory_order_relaxed) % m_capacity;


        T* data = new T[m_baseSize];
            uint32_t n = wPos / m_baseSize;
            uint32_t nPos = wPos % m_baseSize;

        //如果写指针再读指针的右面就应该向后扩展,否则向前扩展
        if (wPos > rPos) {
            m_nodes.insert(m_nodes.begin() + n + 1, data);
        } else {
            m_writePos.store(wPos);//因为向前加入数组并copy 所以位置不变
            m_readPos.store(rPos + m_baseSize);  //注意 rpos可能比wpos大  并不是等于
            m_nodes.insert(m_nodes.begin() + n, data);

            memcpy(m_nodes[n], m_nodes[n + 1], nPos * sizeof(T));
        }

        m_capacity += m_baseSize;
        m_expanding.store(false);
    }

private:
    uint32_t                m_baseSize;
    int			            m_capacity;
    std::vector<T*>          m_nodes;
    std::atomic_int32_t     m_useSize; //这个地方需要用int 不能用uint pop地方需要判断< 0  用来是否判断有数据 
    std::atomic_int32_t     m_useCount; // 用来判断是否需要扩容
    std::atomic_uint32_t    m_readPos;
    std::atomic_uint32_t    m_writePos;
    std::atomic_bool        m_expanding; //正在扩展不能进行pop和expanding
    std::atomic_uint16_t    m_poping; //有几个线程进行pop
    std::atomic_uint16_t    m_pushing;//有几个线程进行push
};
#endif //NLOCKQUEUE_HPP
