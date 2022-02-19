#include "Singleton.hpp"

namespace ray
{
Singleton* Singleton::getInstance()
{
    static Singleton s_singleton;
    return &s_singleton;
}

Singleton::Singleton() {}
}