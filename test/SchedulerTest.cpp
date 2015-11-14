#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Helper.hpp"

#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include "gtest/gtest.h"

using namespace std;

class SchedulerTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	DemoService service_;

	SchedulerTest() : Test(), service_(sched_) {}
	virtual void SetUp() override{
		sched_.start(1);
	}
	virtual void TearDown() override{
		sched_.stop(); 
	}
};

TEST_F(SchedulerTest, callSyncInteface_calledWithinSameContextAsCaller){
	std::string hint("synchronous call");
	thread::id ctxId;
	sched_.interfaceCall("doSomethingA", false, true, [&]() -> bool{
		ctxId = this_thread::get_id();
	}, 2, hint);

	EXPECT_EQ(service_.getResult(), "Method A executed, with parameters:2," + hint + "\n");
	EXPECT_EQ(ctxId, this_thread::get_id());
}

TEST_F(SchedulerTest, callAsyncIntefaceAndBlock_calledUnderDifferentContextWithCaller){
	atomic<bool> called(false);
	thread::id ctxId;
	const size_t consumedMs = 5;

	size_t dur = profileFor([&]{
		sched_.interfaceCall("doSomethingB", true, true, [&]()->bool{
			called = true;
			ctxId = this_thread::get_id();		
			this_thread::sleep_for(chrono::milliseconds(consumedMs));
		}, true, 133);
	});
	
	EXPECT_GT(dur, consumedMs);
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:1,133\n");
	EXPECT_NE(ctxId, this_thread::get_id());
}

TEST_F(SchedulerTest, callAsyncInterfaceInDefaultMode_calledAsynchronouslyWithoutBlocking){
	atomic<bool> called(false);

	sched_.interfaceCall("doSomethingB", true, false, [&]()->bool{
		//std::cout << "Calling B asynchronously done" << std::endl;
		called = true;
	}, false, 131);

	size_t cnt = 0;
	while ((!called) && (cnt++ < 100))
		this_thread::sleep_for(chrono::milliseconds(2));
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:0,131\n");
}

TEST_F(SchedulerTest, asyncCallAfterHeavyAction_AsyncCallDontBlock){
	auto start = chrono::steady_clock::now();
	auto var = make_shared<atomic<int>>(0);
	const size_t heavyActionMs = 5;
	*var = 0;

	size_t dur = profileFor([&]{
		sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
			this_thread::sleep_for(chrono::milliseconds(heavyActionMs));
			*var = 2;
			//cout << "calling1A, var=" << *var << endl;
		}, false, 131);

		sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
			//cout << "calling1B, var=" << *var << endl;
			EXPECT_EQ(*var, 2);
			*var = 4;
		}, false, 111);
	});

	*var = 1;
	EXPECT_LT(dur, heavyActionMs);
	//cout << "case sync end " << endl;
}

TEST_F(SchedulerTest, invokeNotRegisteredCall_NothingCalled){
	//TODO: catch exception and check the throw behavior - shall have assertion error!
	EXPECT_FALSE(sched_.interfaceCall("unknown"));
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}