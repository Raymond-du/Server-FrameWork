#ifndef __UTIL_AOP_HPP
#define __UTIL_AOP_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
// true_type是一个类型
// 使用了ture_type 已经返回ture 值了
// decltype两个参数可以认为  当第一个参数有效时, 返回第二个参数的类型
//
namespace ray
{
#define HAS_MEMBER(member)                                                                    \
    template <typename T, typename... Args>                                                   \
    struct has_func_##member {                                                                \
    private:                                                                                  \
        template <typename U>                                                                 \
        static auto Check(int)                                                                \
            -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type()); \
        template <typename U>                                                                 \
        static auto Check(...) -> std::false_type;                                            \
                                                                                              \
    public:                                                                                   \
        enum { value = std::is_same<decltype(Check<T>(0)), std::true_type>::value };          \
    }

HAS_MEMBER(Before);
HAS_MEMBER(After);

template <class... AspListType>
class AspectList
{
};

template <class Func, class... Args>
class Aspect
{
public:
    Aspect(Func&& func)
        : m_func(std::forward<Func>(func))
    {
    }

    template <class AspType>
    void Invoke(AspType& aspect, Args&&... args)
    {
        Invoke_Before(aspect, std::forward<Args>(args)...);
        m_func(std::forward<Args>(args)...);
        Invoke_After(aspect, std::forward<Args>(args)...);
    }

    template <size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if_t<I == sizeof...(Tp)> for_order(std::tuple<Tp...>&, FuncT)
    {
    }

    template <size_t I = 0, typename FuncT, typename... Tp>
        inline typename std::enable_if_t < I<sizeof...(Tp)> for_order(std::tuple<Tp...>& t, FuncT f)
    {
        f(std::get<I>(t));
        for_order<I + 1, FuncT, Tp...>(t, f);
    }

    template <size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if_t<I == sizeof...(Tp)> for_reverse(std::tuple<Tp...>&, FuncT)
    {
    }

    template <size_t I = 0, typename FuncT, typename... Tp>
        inline typename std::enable_if_t <
        I<sizeof...(Tp)> for_reverse(std::tuple<Tp...>& t, FuncT f)
    {
        for_reverse<I + 1, FuncT, Tp...>(t, f);
        f(std::get<I>(t));
    }

    template <class... AspType>
    void Invoke(std::tuple<AspType...>& aspects, Args&&... args)
    {
        for_order(
            aspects, [this, args...](auto x) { Invoke_Before(x, std::forward<Args>(args)...); });
        m_func(std::forward<Args>(args)...);
        for_reverse(
            aspects, [this, args...](auto x) { Invoke_After(x, std::forward<Args>(args)...); });
    }

private:
    template <class AspType>
    inline typename std::enable_if<has_func_Before<AspType, Args...>::value>::type
    Invoke_Before(AspType& aspect, Args&&... args)
    {
        aspect.Before(std::forward<Args>(args)...);
    }

    template <class AspType>
    inline typename std::enable_if<!has_func_Before<AspType, Args...>::value>::type
    Invoke_Before(AspType&, Args&&...)
    {
    }

    template <class AspType>
    inline typename std::enable_if<has_func_After<AspType, Args...>::value>::type
    Invoke_After(AspType& aspect, Args&&... args)
    {
        aspect.After(std::forward<Args>(args)...);
    }

    template <class AspType>
    inline typename std::enable_if<!has_func_After<AspType, Args...>::value>::type
    Invoke_After(AspType&, Args&&...)
    {
    }

private:
    Func m_func;
};

template <class AspType, class Func, class... Args>
void Invoke(AspType& aspect, Func func, Args&&... args)
{
    Aspect<Func, Args...> asp(std::forward<Func>(func));
    asp.Invoke(aspect, std::forward<Args>(args)...);
}

template <class... AspType, class Func, class... Args>
void Invoke(std::tuple<AspType...> aspects, Func func, Args&&... args)
{
    Aspect<Func, Args...> asp(std::forward<Func>(func));
    asp.Invoke(aspects, std::forward<Args>(args)...);
}
}

#endif //__UTIL_AOP_HPP