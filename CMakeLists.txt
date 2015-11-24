cmake_minimum_required(VERSION 2.8)
# try to detect installed SDK if on LINSEE/build server/etc
if(${CMAKE_HOST_SYSTEM_NAME} EQUAL CYGWIN)
    set(sdkDetected OFF)
    message("use native compiler by default, overriding by useSDK=ON")
else()
    set(lfsVersion "FB_PS_LFS_REL_2015_10_0083")
    set(gccPrefix "/build/ltesdkroot/Platforms/LINUX/${lfsVersion}/sdk/bld-tools/x86_64-pc-linux-gnu")
    set(CMAKE_C_COMPILER ${gccPrefix}/bin/gcc)
    set(CMAKE_CXX_COMPILER ${gccPrefix}/bin/g++)
    set(sdkDetected ON)
    message("use sdk(${lfsVersion}) to build by default, overriding by useSDK=OFF")
endif()
option(useSDK ${sdkDetected})

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
include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(libSrcs
    src/InterfaceScheduler.cpp
    src/Worker.cpp
    src/Helper.cpp
)
add_library(serviceLib STATIC ${libSrcs})
target_link_libraries(serviceLib pthread)

set(testSrcs
	test/SchedulerTest.cpp
    test/SchedulerRegistrationTest.cpp
	test/SchedulePoolTest.cpp
	test/SchedulerUT.cpp
	test/testMain.cpp
	test/WorkerMock.cpp
	test/SchedulePoolTestFixture.cpp
    test/SchedulerMockerTest.cpp
)

add_executable(ut ${testSrcs})
include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ut pthread gtest serviceLib)

add_custom_target(copyFiles COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/runUT.sh ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(runStress COMMAND bash runUT.sh 100 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ut copyFiles)
add_custom_target(runUT COMMAND bash runUT.sh 1 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ut copyFiles)

# generate plantuml documentations
add_custom_target(genDocs COMMAND plant -Sdpi=150 *.pu WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs)

add_executable(demo demo/demo.cpp)
target_link_libraries(demo pthread serviceLib)