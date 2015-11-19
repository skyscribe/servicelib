#include "SchedulerTestFixture.hpp"
#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include <string>
using namespace std;

TEST_F(SchedulerTest, callSyncInteface_calledWithinSameContextAsCaller){
	std::string hint("synchronous call");
	thread::id ctxId;
	CallProperty prop = {false, true, [&]() -> bool{
		ctxId = this_thread::get_id();
	}, ""};
	sched_.interfaceCall("doSomethingA", std::forward<CallProperty>(prop), 2, hint);

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
			//cout << "SyncJob callback happen!" << endl;
		}, "", true, 133);
	});
	
	//EXPECT_GT(dur, consumedMs);
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:1,133\n");
	EXPECT_NE(ctxId, this_thread::get_id());
}

TEST_F(SchedulerTest, callAsyncInterfaceInDefaultMode_calledAsynchronouslyWithoutBlocking){
	atomic<bool> called(false);

	sched_.interfaceCall("doSomethingB", true, false, [&]()->bool{
		//std::cout << "Calling B asynchronously done" << std::endl;
		called = true;
	}, "", false, 131);

	size_t cnt = 0;
	while ((!called) && (cnt++ < 100))
		this_thread::sleep_for(chrono::milliseconds(2));
	EXPECT_TRUE(called);
	EXPECT_EQ(service_.getResult(), "Method B executed with parameters:0,131\n");
}

TEST_F(SchedulerTest, asyncCallAfterHeavyAction_AsyncCallDontBlock){
	auto start = chrono::steady_clock::now();
	auto var = make_shared<atomic<int>>(0);
	const size_t heavyActionMs = 20;
	*var = 0;

	size_t dur = profileFor([&]{
		sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
			this_thread::sleep_for(chrono::milliseconds(heavyActionMs));
			*var = 2;
			//cout << "calling1A, var=" << *var << endl;
		}, "", false, 131);

		sched_.interfaceCall("doSomethingB", true, false, [=]() -> bool{
			//cout << "calling1B, var=" << *var << endl;
			EXPECT_EQ(*var, 2);
			*var = 4;
		}, "", false, 111);
	});

	*var = 1;
	EXPECT_LT(dur, heavyActionMs);
}

//Need stress test for concurrency check!
TEST_F(SchedulerTest, concurrentRegister_registeredSuccessfully){
	InterfaceScheduler sched;
	atomic<int> registered(0);

	auto service = [&](const ParamArgs<int>& par){ registered += get<0>(par); return true;};
	std::vector<shared_ptr<thread>> threads;
	const size_t jobs = 10;

	sched.start(1);
	for (auto i = 0; i < jobs; ++i)
		threads.push_back(make_shared<thread>([&sched, &service, i]() -> void{
			stringstream strm;
			strm << "serv" << i; //to_string not supported on cygwin?
			registerInterfaceFor<int>(sched, strm.str(), service);
			CallProperty prop = {true, true, Callable(), ""};
			EXPECT_TRUE(sched.interfaceCall(strm.str(), forward<CallProperty>(prop), i));
		}));
	for (auto inst : threads)
		inst->join();

	auto getAccumulation = [](int start, int end)->int{
		int result = 0;
		for (auto i = start; i < end; ++i)
			result += i;
		return result;
	};
	EXPECT_EQ(registered, getAccumulation(0, jobs));
}

/* Below code won't pass under C++11
class A{};
class B : public A{};
void func(shared_ptr<B>){}
TEST(testFunc, testFunc_conv){
	std::function<void(shared_ptr<A>)> afunc;
	afunc = func;
}*/