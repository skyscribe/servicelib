cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

add_executable(testService 
	InterfaceScheduler.cpp TestService.cpp Worker.cpp
)