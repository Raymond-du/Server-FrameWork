#ifndef __UTIL_MESSAGEBUS_HPP
#define __UTIL_MESSAGEBUS_HPP

#include "Any.hpp"
#include "NoCopyable.hpp"
#include <functional>
#include <map>
#include <utility>
namespace ray
{
template <class T>
struct Function_Traits;

//普通函数转换为stl::function
template <class Ret, class... Args>
struct Function_Traits<Ret (&)(Args...)> {
    enum { arity = sizeof...(Args) };                 //获取参数的个数
    typedef Ret(function_nomal)(Args...);             // 普通函数的类型
    typedef Ret (*function_point)(Args...);           //函数指针类型
    using function_stl = std::function<Ret(Args...)>; // stl function类型

    template <size_t I>
    struct Param {
        //获取第I个参数的类型
        static_assert(I < arity, "index is out of range");
        //通过tuple 获取参数类型
        using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
    };
};

//函数指针
template <class Ret, class... Args>
struct Function_Traits<Ret (*)(Args...)> : Function_Traits<Ret (&)(Args...)> {
};

// stl函数
template <class Ret, class... Args>
struct Function_Traits<std::function<Ret(Args...)>> : Function_Traits<Ret (&)(Args...)> {
};

//成员函数 成员函数可能包含 const volatile的修饰符
#define FUNCTION_TRAITS(...)                                        \
    template <class Ret, class ClassType, class... Args>            \
    struct Function_Traits<Ret (ClassType::*)(Args...) __VA_ARGS__> \
        : Function_Traits<Ret (&)(Args...)> {                       \
    };

FUNCTION_TRAITS();
FUNCTION_TRAITS(const);
FUNCTION_TRAITS(volatile);
FUNCTION_TRAITS(const volatile);
#undef FUNCTION_TRAITS
//成员函数
template <class ClzFunc>
struct Function_Traits : Function_Traits<decltype(&ClzFunc::operator())> {
};

//如果没有该方法 const Func则不能使用lamabd, 因为右值引用不能包含 const Func&
template <class Func>
typename Function_Traits<Func>::function_stl to_stl_function(const Func& func)
{
    return static_cast<typename Function_Traits<Func>::function_stl>(func);
}

template <class Func>
typename Function_Traits<Func>::function_stl to_stl_funtion(Func&& func)
{
    return static_cast<typename Function_Traits<Func>::function_stl>(func);
}

class MessageBus : public NoCopyable
{
public:
    template <class Func>
    void addObserver(Func&& func, const std::string& msgTopic = "")
    {
        auto observerFunc = to_stl_funtion(std::forward<Func>(func));
        std::string topic;
        topic.append(msgTopic).append(typeid(observerFunc).name());
        add_(topic, std::move(observerFunc));
    }
#define ADDOBSERVER(...)                                                             \
    template <class Ret, class ClassType, class... Args>                             \
    void addObserver(ClassType* obj, Ret (ClassType::*clzFunc)(Args...) __VA_ARGS__, \
        const std::string& msgTopic = "")                                            \
    {                                                                                \
        std::function<Ret(Args...)> observerFunc = [obj, clzFunc](Args... args) {    \
            (*obj.*clzFunc)(std::forward<Args>(args)...);                            \
        };                                                                           \
        addObserver(std::move(observerFunc), msgTopic);                              \
    }

    ADDOBSERVER()
    ADDOBSERVER(const)
    ADDOBSERVER(volatile)
    ADDOBSERVER(const volatile)
#undef ADDOBSERVER

    template <class Ret, class... Args>
    void notyfyMsg(const std::string& msgTopic, Args... args)
    {
        using observerFunc = typename Function_Traits<Ret (&)(Args...)>::function_stl;
        std::string topic;
        topic.append(msgTopic).append(typeid(observerFunc).name());
        auto range = m_observers.equal_range(topic);
        for(auto iter = range.first; iter != range.second; ++iter) {
            auto f = iter->second.get<observerFunc>();
            f(std::forward<Args>(args)...);
        }
    }

private:
    void add_(const std::string& topic, Any any)
    {
        m_observers.insert(std::pair<std::string, Any>(topic, any));
    }

private:
    std::multimap<std::string, Any> m_observers;
};
}

#endif //__UTIL_MESSAGEBUS_HPP
