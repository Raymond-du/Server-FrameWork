
#ifndef __MACRO_H
#define __MACRO_H

#include <assert.h>
#include "Logger.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
//告诉编译器优化,条件大概率成立
#define RAYMOND_LIKELY(x) __builtin_expect(!!(x), 1)
#define RAYMOND_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else 
#define RAYMOND_LIKELY(x)	(x)
#define RAYMOND_UNLIKELY(x)	(x)


#endif

#define RAYMOND_ASSERT(x) \
    if(RAYMOND_LIKELY(!(x))) { \
    RAYMOND_RLOG_ERROR("ASSERTION : %s\nbacktrace : %s\n",#x,raymond::backTraceToString().c_str());  \
    assert(x);    \
    } 
#define RAYMOND_ASSERT2(x,w) \
    if(RAYMOND_UNLIKELY(!(x))) { \
	RAYMOND_RLOG_ERROR("ASSERTION : %s    %s\nbacktrace : %s\n",#x,w,raymond::backTraceToString().c_str());  \
	assert(x);    \
    } 
#endif

//请求的宏定义
#define REQUEST_LOGIN 100
#define REQUEST_REG		101
