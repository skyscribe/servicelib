cmake_minimum_required(VERSION 2.8) 
if(CMAKE_VERSION VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
else()
set (CMAKE_CXX_STANDARD 11)
endif()

#google test/mock
set(gtestDir ${CMAKE_CURRENT_SOURCE_DIR}/gtest/)
add_library(gtest STATIC ${gtestDir}/src/gtest-all.cc) 
include_directories(SYSTEM PUBLIC ${gtestDir}/include)
include_directories(PUBLIC ${gtestDir})

set(srcList
	src/InterfaceScheduler.cpp
	src/Worker.cpp
	test/SchedulerTest.cpp
	test/SchedulePoolTest.cpp
)

add_executable(ut ${srcList})
include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ut pthread gtest)

add_custom_target(copyFiles COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/runUT.sh ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(runUT COMMAND bash runUT.sh WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ut copyFiles)