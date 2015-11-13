#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Worker.h" //required for unique_ptr
#include <iostream>
#include "gtest/gtest.h"

using namespace std;

class SchedulerTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	DemoService service_;

	SchedulerTest() : Test(), service_(sched_) {}
	virtual void SetUp(){
		sched_.start();
	}
	virtual void TearDown(){
		sched_.stop(); 
		//cout << "stopped!" << endl;
	}
};

TEST_F(SchedulerTest, callSyncInteface){
	std::string hint("synchronous call");
	sched_.interfaceCall("doSomethingA", false, true, Callable(), 2, hint);	
	EXPECT_EQ(service_.getResult(), "Method A executed, with parameters:2," + hint + "\n");
}

TEST_F(SchedulerTest, callSyncIntefaceViaAsync){
	atomic<bool> called(false);
	sched_.interfaceCall("doSomethingB", true, true, [&]()->bool{
		called = true;
	}, true, 133);
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:1,133\n");
}

TEST_F(SchedulerTest, callAsyncInterface){
	atomic<bool> called(false);

	sched_.interfaceCall("doSomethingB", true, false, [&]()->bool{
		//std::cout << "Calling B asynchronously done" << std::endl;
		called = true;
	}, false, 131);

	size_t cnt = 0;
	while ((!called) && (cnt++ < 100))
		this_thread::sleep_for(chrono::milliseconds(10));
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:0,131\n");
}

TEST_F(SchedulerTest, asyncExecutionDontBlockNewAsynCall){
	auto start = chrono::steady_clock::now();
	auto var = make_shared<atomic<int>>(0);
	*var = 0;
	sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
		this_thread::sleep_for(chrono::milliseconds(20));
		*var = 2;
		//cout << "calling1A, var=" << *var << endl;
	}, false, 131);

	sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
		//cout << "calling1B, var=" << *var << endl;
		EXPECT_EQ(*var, 2);
		*var = 4;
	}, false, 111);
	
	*var = 1;
	auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
	EXPECT_LT(diff.count(), 200);
	//cout << "case sync end " << endl;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}