#ifndef __NET_NOCOPYABLE_HPP
#define __NET_NOCOPYABLE_HPP

namespace ray
{
class NoCopyable
{
public:
    NoCopyable() = default;
    NoCopyable(const NoCopyable&) = delete;
    virtual ~NoCopyable() = default;
    const NoCopyable& operator=(const NoCopyable&) = delete;
};
}

#endif