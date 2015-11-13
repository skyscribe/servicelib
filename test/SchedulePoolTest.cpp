#include "InterfaceScheduler.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include "gtest/gtest.h"
using namespace std;

const size_t timeForSingleJob = 10;
atomic<int> sharedVar(0);
auto timeConsumingJob = [](const ParamArgs<int>& idx) -> bool{
	this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
	sharedVar += get<0>(idx);
};

class SchedulePoolTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	const size_t poolSize_ = 8;

	virtual void SetUp() override{
		sched_.registerInterface("service", [](const ParaArgsBase& p) -> bool {
			return timeConsumingJob(static_cast<const ParamArgs<int>&>(p));
		});
		sched_.start(poolSize_);
	}

	virtual void TearDown() override{
		sched_.stop();
	}
};

TEST_F(SchedulePoolTest, jobsDistributedEvenly){
	atomic<int> done(0);
	const size_t jobs = 6;
	auto start = chrono::steady_clock::now();

	for (auto i = 0; i < jobs; ++i){
		sched_.interfaceCall("service", true, false, [&]() -> bool{
			done++;
		}, 1 << i);
	}

	while (done != jobs)
		this_thread::sleep_for(chrono::milliseconds(10));

	auto diff = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
	cout << "Thread pool used " << static_cast<float>(diff.count()/timeForSingleJob) << " of single run to finish " 
		<< jobs << " jobs, elapsed=" << diff.count() << endl; 
	EXPECT_LT(diff.count(), timeForSingleJob*jobs);
	EXPECT_EQ(sharedVar, (1 << jobs) - 1);	
}