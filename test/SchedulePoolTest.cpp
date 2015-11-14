#include "InterfaceScheduler.hpp"

#include "Helper.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>

#include "gtest/gtest.h"

using namespace std;

class SchedulePoolTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	const size_t poolSize_ = 4;
	atomic<int> sharedVar_;
	const size_t timeForSingleJob = 20;

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

TEST_F(SchedulePoolTest, schedulePoolSizeJobs_jobsDistributedEvenly){
	atomic<int> done(0);
	const size_t jobs = poolSize_;
	mutex lock;
	set<thread::id> idsList;
	std::stringstream desc;
	desc << "Launching " << jobs << " jobs asynchronously [each requires{" << timeForSingleJob << "}ms]";

	auto tm = profileFor([&]{
		for (auto i = 0; i < jobs; ++i){
			sched_.interfaceCall("service", true, false, [&]() -> bool{
				done++;
				lock.lock();
				idsList.insert(this_thread::get_id());
				lock.unlock();
			}, 1 << i);
		}

		size_t i = 0;
		for(; (i < 1000) && (done != jobs); ++i)
			this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
	}, desc.str(), false /*true*/);

	EXPECT_EQ(done, jobs);
	EXPECT_LT(tm, timeForSingleJob*jobs);
	EXPECT_EQ(idsList.size(), jobs);
	EXPECT_EQ(sharedVar_, (1 << jobs) - 1);	
}