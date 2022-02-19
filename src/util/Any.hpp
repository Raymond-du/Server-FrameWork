#ifndef __UTIL_ANY_HPP
#define __UTIL_ANY_HPP

#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace ray
{
class Any
{
public:
    Any()
        : m_tpIndex(std::type_index(typeid(void)))
        , m_ptr(nullptr)
    {
    }

    Any(const Any& other)
        : m_tpIndex(other.m_tpIndex)
    {
        m_ptr = other.m_ptr->clone();
    }

    Any(Any&& other)
        : m_tpIndex(other.m_tpIndex)
    {
        other.m_tpIndex = std::type_index(typeid(void));
        m_ptr = std::move(other.m_ptr);
    }

    template <class T,
        class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value,
            T>::type>
    Any(T&& value)
        : m_ptr(new Drived<typename std::decay<T>::type>(std::forward<T>(value)))
        , m_tpIndex(std::type_index(typeid(T)))
    {
    }

    template <class T, class... Args>
    void reset(Args&&... args)
    {
        m_tpIndex = std::type_index(typeid(T));
        m_ptr.reset(new Drived<T>(std::forward<Args>(args)...));
    }

    template <class T,
        class =
            typename std::enable_if<!std::is_same<typename std::decay<T>::type, Any>::value, T>::type>
    void operator=(T&& value)
    {
        m_tpIndex = std::type_index(typeid(T));
        m_ptr.reset(new Drived<T>(std::forward<T>(value)));
    }
    
    void operator=(Any&& other)
    {
        m_tpIndex = other.m_tpIndex;
        m_ptr = std::move(other.m_ptr);
    }
    
    void operator=(const Any& other)
    {
        m_tpIndex = other.m_tpIndex;
        m_ptr = other.m_ptr->clone();
    }

    template <class T>
    bool isType()
    {
        return m_tpIndex == std::type_index(typeid(T));
    }

    template <class T>
    T& get()
    {
        if(isType<T>()) {
            return dynamic_cast<Drived<T>*>(m_ptr.get())->m_value;
        } else {
            throw "type is missmatched";
        }
    }

private:
    struct Base;
    using BasePtr = std::unique_ptr<Base>;
    struct Base {
        virtual ~Base() = default;
        virtual BasePtr clone() = 0;
    };

    template <class T>
    struct Drived : Base {
        template <class... Args>
        Drived(Args&&... args)
            : m_value(std::forward<Args>(args)...)
        {
        }
        BasePtr clone() override { return std::unique_ptr<Base>(new Drived<T>(m_value)); }

        T m_value;
    };

    BasePtr m_ptr;
    std::type_index m_tpIndex;
};
}

#endif //__UTIL_ANY_HPP
