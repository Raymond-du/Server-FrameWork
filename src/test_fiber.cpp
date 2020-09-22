#include "Logger.h"
#include "fiber.h"
#include "raymond.h"

void func(){
    RAYMOND_RLOG_DEBUG("run in func");
//    raymond::Fiber::getCurFiber()->swapOut();
//    RAYMOND_RLOG_DEBUG("run continue in func");
}

void func2(){
    RAYMOND_RLOG_DEBUG("run in func");
    raymond::Fiber::getCurFiber()->swapOut();
    RAYMOND_RLOG_DEBUG("run continue in func");
}

int main(){
 //   raymond::Fiber::getCurFiber();
    raymond::Fiber::ptr fiber(new raymond::Fiber(&func));
    RAYMOND_RLOG_DEBUG("main run ");
    fiber->swapIn();
    RAYMOND_RLOG_DEBUG("main continue");
    return 0;

}
