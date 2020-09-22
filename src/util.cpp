
#include "util.h"
#include "Logger.h"
#include <bits/stdint-uintn.h>
#include <bits/types/struct_timeval.h>
#include <functional>
#include <future>
#include <thread>
#include <typeinfo>
#include <utility>
#include <sys/time.h>

namespace raymond{

    //获取系统的日志打印器
    auto logger = RAYMOND_LOG_BYNAME("system");

	static std::string demangle(const char* str) {
		size_t size = 0;
		int status = 0;
		std::string rt;
		rt.resize(256);
		if(1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
			char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
			if(v) {
				std::string result(v);
				free(v);
				return result;
			}
		}
		if(1 == sscanf(str, "%255s", &rt[0])) {
			return rt;
		}
		return str;
	}

    void backTraceToVec(std::vector<std::string>& vec, size_t size  , size_t skip ){
	    //获取size 个指针   指针的大小为int
	    void  **  buf = (void **)malloc(sizeof(void*) * size); 
	    //获取 栈空间信息 数据
	    size_t s = backtrace(buf,size);

	    // 解析栈信息数据 到 字符串
	    char ** strings =  backtrace_symbols(buf,s);
	    if(strings == nullptr){
		    RAYMOND_LOG_FMT_ERROR(logger,"backtrace_sysmbols error");
		    return;
	    }
	    for (size_t i = skip; i < s; i++)
	    {                        
		   vec.push_back(demangle(strings[i]));                       
	    }
	    free(buf);
	    free(strings);
    }

    std::string backTraceToString(size_t size ,size_t skip ,const std::string & separator){
	    std::vector<std::string> vec;
	    std::stringstream ss;

	    backTraceToVec(vec,size,skip);

	    for(auto s : vec){
		    ss<<s<<separator;
	    }
	    return ss.str();
    }
    
	timeval getCurTime(){
		::timeval tv;
		::gettimeofday(&tv,nullptr);
		return tv;
	}

	uint32_t getCurMs(){
		::timeval tv;
		::gettimeofday(&tv,nullptr);
		return tv.tv_sec * 1000ul+ tv.tv_usec/1000ul;
	}

	uint32_t getCoreNum(){
		return std::thread::hardware_concurrency();
	}	
}
