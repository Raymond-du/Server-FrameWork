#include "thread.h"
#include <iostream>
#include <chrono>

void func(){
        std::string name = raymond::Thread::getCurName();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"nihk"<<name <<std::endl;
}

// int main(){
//         // raymond::Thread a("thread 1",[]{
//         //         std::string name = raymond::Thread::getCurName();
//         //         std::cout<<"nihk"<<name <<std::endl;
//         // });
//         {

//         raymond::Thread a("raymond-du",func);
//         a.detach();
//         std::this_thread::sleep_for(std::chrono::seconds(2));

//         }
// }