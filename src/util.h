#ifndef __UNIT_H
#define __UNIT_H 

#include <bits/stdint-uintn.h>
#include <cxxabi.h>
#include <future>
#include <memory>
#include <string>
#include <vector>
#include <execinfo.h>
#include <functional>
#include <thread>

namespace raymond{
        /**
         * @brief 获取类型的类型名
         * 
         * @tparam T    泛型
         * @return      类型名
         */
        template <class T>
        std::string typeToName(){
                char* cs = abi::__cxa_demangle(typeid(T).name(),nullptr,nullptr,nullptr);
                std::string str (cs);
                free(cs);
                return str;
        }

        /**
         * @brief  获取当前的调用栈信息
         *              
         * @param vec   返回栈信息
         * @param size   保存栈的长度 默认24
         * @param skip  跳过保存栈信息的个数
         */
        void backTraceToVec(std::vector<std::string>& vec, size_t size = 24 , size_t skip = 1);
        /**
         * @brief       获取当前的调用栈信息
         * 
         * @param size   保存栈的长度 默认24
         * @param skip  跳过保存栈信息的个数
         * @param separator     分隔符
         * @return      调用找的信息(字符串)
         */
        std::string backTraceToString(size_t size = 24,size_t skip = 2,const std::string & separator = "\n");
	
	template<class F,class ...Args>
	std::function<void()> formatFunc(F && func,Args&& ...args){
	    using result_type = typename std::result_of<F(Args...)>::type;
	    auto task = std::make_shared<std::packaged_task<result_type()>> (
		(std::bind(std::forward<F>(func),std::forward<Args>(args)...))); 
	    std::function<void()> f = [task]{
		(*task)();
	    };
	    return f;
	}

	/**
	 * @brief getCurMs 获取当前的时间(毫秒精度)
	 *
	 * @return 时间
	 */
	timeval getCurTime();

	uint32_t getCurMs();

	/**
	 * @brief getCoreNum 获取CPU核心数
	 *
	 * @return 核心数
	 */
	uint32_t getCoreNum();

	/**
	 * @brief split 字符串的分割
	 *	
	 * @param str	需要分割的字符串
	 * @param delim	分隔符
	 *
	 * @return 返回分割后的字符串数组
	 */
	std::vector<std::string> split(const std::string& str,const std::string& delim);

	/**
	 * @brief operator<< 重载<<运算符appStr字符串追加到src的后面
	 *
	 * @param src	原字符串
	 * @param appStr 欲添加的字符串
	 *
	 * @return 返回原字符串的引用
	 * */
	std::string& operator<< (std::string& src, const std::string& appStr);
}

#endif	// __UNIT_H
