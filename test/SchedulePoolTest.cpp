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

	std::mutex lock_;
	std::vector<std::pair<int, thread::id>> threadMapping_;

	virtual void SetUp() override{
		sharedVar_ = 0;
		sched_.registerInterface("heavy", [&](const ParaArgsBase& p) -> bool {
			this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
			sharedVar_ += get<0>(static_cast<const ParamArgs<int>&>(p));
			return true;
		});
		registerInterfaceFor<int>(sched_, "light", [&](const ParamArgs<int> p)->bool{
			lock_.lock();
			threadMapping_.push_back({get<0>(p), this_thread::get_id()});
			lock_.unlock();
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
			sched_.interfaceCall("heavy", true, false, [&]() -> bool{
				lock.lock();
				idsList.insert(this_thread::get_id());
				lock.unlock();
				done++;
			}, "", 1 << i);
		}
	}, desc.str(), true /*true*/);

	auto waitTm = profileFor([&]{
	size_t i = 0;
	for(; (i < 1000) && (done != jobs); ++i)
		this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
	}, "", false);

	ASSERT_EQ(done, jobs);
	ASSERT_LT(tm, timeForSingleJob);
	EXPECT_LT(tm + waitTm, timeForSingleJob*jobs);
	EXPECT_EQ(idsList.size(), jobs);
	EXPECT_EQ(sharedVar_, (1 << jobs) - 1);	
}

TEST_F(SchedulePoolTest, scheduleWithAffinity_allJobsRunAsStrand){
	atomic<int> done(0);
	const size_t jobs = poolSize_*2;

	auto tm = profileFor([&]{
		for (auto i = 0; i < jobs; ++i)
			sched_.interfaceCall("light", true, false, [&]() -> bool{
				done++;
			}, "strand", 1 << i);
		while (done != jobs)
			this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
	}, "", false);

	EXPECT_EQ(threadMapping_.size(), jobs);
	std::set<thread::id> idSets;
	for (auto item : threadMapping_)
		idSets.insert(item.second);
	EXPECT_EQ(idSets.size(), 1);
}