#include "SignalCapture.hpp"
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <signal.h>

std::unordered_map<int, std::function<void()>> g_captureFuncs;

void g_signal_deal_func(int signalId)
{
	auto iter = g_captureFuncs.find(signalId);
	if (iter != g_captureFuncs.end()) {
		iter->second();
	} else {
		std::exit(0);
	}
}

namespace ray
{
int SignalCapture(int signalId, std::function<void()> func)
{
    int ret;
    struct sigaction act;
    act.sa_handler = &g_signal_deal_func;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	ret = sigaction(signalId, &act, nullptr);
    if (ret != 0) {
        return ret;
    }
	g_captureFuncs[signalId] = func;
    return ret;
}

}
