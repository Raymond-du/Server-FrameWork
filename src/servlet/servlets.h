#ifndef __SERVLET_SERVLETS_H
#define __SERVLET_SERVLETS_H

#include "../json/json/json.h"
#include <bits/stdint-intn.h>
namespace raymond {
	
	int32_t loginFunc(Json::Value& request, Json::Value& response);
	int32_t registFunc(Json::Value& request, Json::Value& response);
}

#endif
