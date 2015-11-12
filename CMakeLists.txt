cmake_minimum_required(VERSION 2.8) 
if(CMAKE_VERSION VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
else()
	set (CMAKE_CXX_STANDARD 11)
endif()

add_executable(testService 
	InterfaceScheduler.cpp TestService.cpp Worker.cpp
)
target_link_libraries(testService pthread)