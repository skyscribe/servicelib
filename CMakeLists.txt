cmake_minimum_required(VERSION 2.8)
if(${CMAKE_HOST_SYSTEM_NAME} EQUAL CYGWIN)
    message("use native compiler")
else()
    message("use sdk to build")
    set(gccPrefix "/build/ltesdkroot/Platforms/LINUX/FB_PS_LFS_REL_2015_10_0083/sdk/bld-tools/x86_64-pc-linux-gnu")
    set(CMAKE_C_COMPILER ${gccPrefix}/bin/gcc)
    set(CMAKE_CXX_COMPILER ${gccPrefix}/bin/g++)
endif()


if(CMAKE_VERSION VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
else()
set (CMAKE_CXX_STANDARD 11)
endif()

#google test/mock
set(gtestDir ${CMAKE_CURRENT_SOURCE_DIR})
add_library(gtest STATIC ${gtestDir}/gtest/src/gtest-all.cc ${gtestDir}/gmock/src/gmock-all.cc) 
include_directories(SYSTEM PUBLIC ${gtestDir}/gtest/include ${gtestDir}/gmock/include)
include_directories(PUBLIC ${gtestDir}/gtest ${gtestDir}/gmock)

set(srcList
	src/InterfaceScheduler.cpp
	src/Worker.cpp
	test/SchedulerTest.cpp
	test/SchedulePoolTest.cpp
	test/SchedulerUT.cpp
	test/testMain.cpp
	test/WorkerMock.cpp
	test/SchedulePoolTestFixture.cpp
)

add_executable(ut ${srcList})
include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ut pthread gtest)

add_custom_target(copyFiles COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/runUT.sh ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(runStress COMMAND bash runUT.sh 100 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ut copyFiles)
add_custom_target(runUT COMMAND bash runUT.sh 1 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ut copyFiles)