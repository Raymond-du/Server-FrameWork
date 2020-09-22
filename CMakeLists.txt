cmake_minimum_required(VERSION 3.0)
project(program)
#aux_source_directory(./src DIR_TOOT_SRCS)

set(CMAKE_VERBOSE_MAKEFILE on)


set(DIR_TOOT_SRCS 
        ./src/config.cpp
        ./src/Logger.cpp
        ./src/Semaphore.cpp
        ./src/thread.cpp
        ./src/util.cpp
		./src/fiber.cpp
		./src/schedule.cpp
		./src/ThreadPool.cpp
		./src/iomanager.cpp
		./src/timer1.cpp
		./src/hook.cpp
	)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -g ")


#add_executable(${PROJECT_NAME} ./src/test_fiber.cpp ${DIR_TOOT_SRCS})
#add_executable(${PROJECT_NAME} ./src/test.cpp ${DIR_TOOT_SRCS})

#add_executable(${PROJECT_NAME} ./test/test_schedule.cpp ${DIR_TOOT_SRCS})
#add_executable(${PROJECT_NAME} ./test/test.cpp ${DIR_TOOT_SRCS})

#add_executable(${PROJECT_NAME} ./test/test_sConnection.cpp ${DIR_TOOT_SRCS})

add_executable(${PROJECT_NAME} ./test/test_hook.cpp ${DIR_TOOT_SRCS})

target_link_libraries(program /usr/lib/x86_64-linux-gnu/libyaml-cpp.so)
target_link_libraries(program /usr/lib/x86_64-linux-gnu/libpthread.so)
target_link_libraries(program /usr/lib/x86_64-linux-gnu/libdl.so)


