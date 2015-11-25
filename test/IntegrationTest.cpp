#include "IntegrationTestFixture.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include "gmock/gmock.h"

using namespace std;

TEST_F(IntegrationTest, callSyncInteface_calledWithinSameContextAsCaller){
	std::string hint("synchronous call");
	thread::id ctxId;
	CallProperty prop = {false, true, [&]() -> bool{
		ctxId = this_thread::get_id();
	}, ""};
	sched_.interfaceCall("doSomethingA", std::forward<CallProperty>(prop), 2, hint);

	EXPECT_EQ(service_.getResult(), "Method A executed, with parameters:2," + hint + "\n");
	EXPECT_EQ(ctxId, this_thread::get_id());
}

TEST_F(IntegrationTest, callAsyncIntefaceAndBlock_calledUnderDifferentContextWithCaller){
	atomic<bool> called(false);
	thread::id ctxId;
	//const size_t consumedMs = 5;

	size_t dur = profileFor([&]{
		CallProperty prop = {true, true, [&]()->bool{
			called = true;
			ctxId = this_thread::get_id();		
			//this_thread::sleep_for(chrono::milliseconds(consumedMs));
			//cout << "SyncJob callback happen!" << endl;
		}, ""};
		sched_.interfaceCall("doSomethingB", std::forward<CallProperty>(prop), true, 133);
	});
	
	//EXPECT_GT(dur, consumedMs);
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:1,133\n");
	EXPECT_NE(ctxId, this_thread::get_id());
}

TEST_F(IntegrationTest, callAsyncInterfaceInDefaultMode_calledAsynchronouslyWithoutBlocking){
	atomic<bool> called(false);

	sched_.interfaceCall("doSomethingB", createAsyncNonBlockProp([&]()->bool{
		//std::cout << "Calling B asynchronously done" << std::endl;
		called = true;
	}), false, 131);

	while ((!called))
		this_thread::yield();
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:0,131\n");
}

TEST_F(IntegrationTest, asyncCallAfterPreviousCallRunning_AsyncCallDontBlock){
	auto start = chrono::steady_clock::now();
	auto var = make_shared<atomic<int>>(0);

	size_t dur = profileFor([&]{
		sched_.interfaceCall("doSomethingB", createAsyncNonBlockProp([=]() -> bool{
			//This call can wait on condition to be set only after calling site exit interfaceCall
			while ((*var) != 1)
				this_thread::yield();
			*var = 2;
		}), false, 131);

		sched_.interfaceCall("doSomethingB", createAsyncNonBlockProp([=]() -> bool{
			//cout << "calling1B, var=" << *var << endl;
			EXPECT_EQ(*var, 2);
			*var = 3;
		}), false, 111);
	});

	*var = 1; //will be joined during tear down
}

////////////////////////////////////////////////////////////////////////////////
//Type checker
TEST(SchedulerTypeChecker, callWithDifferentParameters_exceptionThrownOnCall){
	atomic<bool> called(0);
	InterfaceScheduler sched;
	registerInterfaceFor<int, int>(sched, "service", [](const ParamArgs<int, int>&){return true;});
	sched.start(1);

	EXPECT_THROW(sched.interfaceCall("service", createAsyncNonBlockProp([&](){
		called = 1;
		return true;
	}), 1), std::invalid_argument);
	sched.stop();

	EXPECT_NE(called, 1); //not called
}

TEST(SchedulerTypeChecker, registerVoidActionAndCall_noExceptionThrown){
	InterfaceScheduler sched;
	ASSERT_EQ(ParamArgs<>::getType(), typeid(std::tuple<>).name());
	EXPECT_NO_THROW(registerInterfaceFor<>(sched, "some service", 
		[](const ParamArgs<>&) -> bool{ return true;}));
}

TEST(SchedulerTypeChecker, registerEmptyAction_registrationFailed){
	InterfaceScheduler sched;
	std::function<bool(const ParamArgs<int>&)> emptyFunc;
	EXPECT_THROW(registerInterfaceFor<int>(sched, "some service",
		emptyFunc), std::invalid_argument);
	//below register won't compile
	//std::function<void(const ParamArgs<int>&)> badFunc;
	//EXPECT_THROW(registerInterfaceFor<int>(sched, "some service",
	//	badFunc), std::invalid_argument);
}