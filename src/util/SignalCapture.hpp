#ifndef __UTIL_SIGNALCAPTURE_HPP
#define __UTIL_SIGNALCAPTURE_HPP
#include <functional>

namespace ray
{

int SignalCapture(int signalId, std::function<void()> func);
}

#endif
