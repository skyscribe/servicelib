#include "gtest/gtest.h"
#include "InterfaceSchedulerMock.hpp"

InterfaceScheduler* createTestScheduler(){
    return new MockedInterfaceScheduler();
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}