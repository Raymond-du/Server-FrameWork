#include <type_traits>
#include <utility>
#include <exception>

namespace ray
{
template<class T>
class optional
{
public:
    using value_type = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;

    optional() : m_isInit(false) {}

    optional(const T& value) : m_isInit(false)
    {
        create(value);
    }

    optional(T&& value) : m_isInit(false)
    {
        create(std::move(value));
    }

    template<class... Args>
    optional(Args&&... param) : m_isInit(false)
    {
        create(std::forward<Args>(param)...);
    }

    optional(const optional& other) : m_isInit(false)
    {
        if (other.m_isInit) {
            copy(other.m_value);
        }
    }

    optional(optional&& other) : m_isInit(false)
    {
        if (other.m_isInit) {
            move(std::move(other.m_value));
            other.destory();
        }
    }

    operator T()
    {
        if (!m_isInit) {
            throw "optional is empty";
        }
        return (*(T*)&m_value);
    }

    explicit operator bool()
    {
        return m_isInit;
    }

    T* operator *()
    {
        if (!m_isInit) {
            throw "optional is empty";
        }
        return (T*)m_value;
    }

    optional& operator =(const optional& other)
    {
        if (m_isInit) {
            destory();
        }
        if (other.m_isInit) {
            copy(other.m_value);
        }
        return *this;
    }

    optional& operator =(optional&& other)
    {
        if (m_isInit) {
            destory();
        }
        if (other.m_isInit) {
            move(std::move(other.m_value));
            other.destory();
        }
        return *this;
    }

    bool isEmpty()
    {
        return m_isInit;
    }

    T& getRef()
    {
        return (*(T*)&m_value);
    }

private:
    template <class... Args>
    void create(Args&&... param)
    {
        new (&m_value) T(std::forward<Args>(param)...);
        m_isInit = true;
    }

    void copy(const value_type& value)
    {
        destory();
        new (&m_value) T((*(T*)&value));
        m_isInit = true;
    }

    void move(value_type&& value)
    {
        destory();
        new (&m_value) T(std::move(*(T*)&value));
        m_isInit = true;
    }

    void destory()
    {
        if (m_isInit) {
            ((T*)&m_value)->~T();
            m_isInit = false;
        }
    }


private:
    bool m_isInit;
    value_type m_value;
};

}
