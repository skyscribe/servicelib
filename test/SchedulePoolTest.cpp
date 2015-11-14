#include "InterfaceScheduler.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include "gtest/gtest.h"
using namespace std;

//profiler function
auto profileFor = [](std::function<void()> call, const std::string& hint){
	auto start = chrono::steady_clock::now();
	call();
	auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);	
	cout << "Time spent on <" << hint << "> is " << diff.count() << " milliseconds" << endl;
};

class SchedulePoolTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	const size_t poolSize_ = 8;
	atomic<int> sharedVar_;
	const size_t timeForSingleJob = 10;

	virtual void SetUp() override{
		sharedVar_ = 0;
		sched_.registerInterface("service", [&](const ParaArgsBase& p) -> bool {
			this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
			sharedVar_ += get<0>(static_cast<const ParamArgs<int>&>(p));
			return true;
		});
		profileFor([&]{
			sched_.start(poolSize_);
		}, "start scheduler");
	}

	virtual void TearDown() override{
		sched_.stop();
	}
};

TEST_F(SchedulePoolTest, jobsDistributedEvenly){
	atomic<int> done(0);
	const size_t jobs = 8;
	auto start = chrono::steady_clock::now();

	for (auto i = 0; i < jobs; ++i){
		sched_.interfaceCall("service", true, false, [&]() -> bool{
			done++;
		}, 1 << i);
	}

	size_t i = 0;
	for(; (i < 1000) && (done != jobs); ++i)
		this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));

	auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
	cout << "Thread pool used " << static_cast<float>(diff.count()/timeForSingleJob) << " of single run to finish " 
		<< jobs << " jobs, elapsed=" << diff.count() << endl; 

	EXPECT_EQ(done, jobs);
	EXPECT_LT(diff.count(), timeForSingleJob*jobs);
	EXPECT_EQ(sharedVar_, (1 << jobs) - 1);	
}