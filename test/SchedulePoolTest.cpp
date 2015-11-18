#include "InterfaceScheduler.hpp"

#include "Helper.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>

#include "WorkerMock.hpp"
#include "SchedulePoolTestFixture.h"
#include "gtest/gtest.h"

using namespace std;
using ::testing::Invoke;
using ::testing::ElementsAre;
using ::testing::_;

auto getExponentOf = [&](int i) -> int{ return 1 << i; };

TEST_F(SchedulePoolTest, schedulePoolSizeJobs_jobsDistributedEvenly){
	const size_t jobs = poolSize_;
	mutex lock;
	set<thread::id> idsList;
	setExpectationForMockedWorker([](AsyncWorkerMock& worker){
		EXPECT_CALL(worker, doJob(_, _)).Times(1);
	});

	auto tmInfo = runAsyncJobsAndWaitForFinish(jobs, "sum", getExponentOf, "", [&](){
			unique_lock<mutex> guard(lock);
			idsList.insert(this_thread::get_id());
		}
	);

	//NOTE: check on running time may not be stable and subject to CPU scheduling!
	//  tmInfo for reference/debug only
	//ASSERT_LT(tmInfo.first, timeForSingleJob);
	//EXPECT_LT(tmInfo.first + tmInfo.second, timeForSingleJob*jobs);
	EXPECT_EQ(idsList.size(), jobs);
	EXPECT_EQ(sharedVar_, (1 << jobs) - 1);
}

TEST_F(SchedulePoolTest, scheduleWithAffinity_allJobsRunAsAStrand){
	const size_t jobs = poolSize_*2;

	setExpectationForMockedWorker([](AsyncWorkerMock& worker){
		EXPECT_CALL(worker, doJob(_, _)).Times(0);
	});
	EXPECT_CALL(*(static_cast<AsyncWorkerMock*>(workers_[0].get())), 
		doJob(_, _)).Times(jobs);

	auto tmInfo = runAsyncJobsAndWaitForFinish(jobs, "collect", getExponentOf, "strand");

	EXPECT_EQ(threadMapping_.size(), jobs);
	std::set<thread::id> idSets;
	for (auto item : threadMapping_)
		idSets.insert(item.second);
	EXPECT_EQ(idSets.size(), 1);
}

TEST_F(SchedulePoolTest, scheduleOnInterleavedStrand_allJobsRunOnAssociatedStrands){
	const size_t jobs = poolSize_;

	setExpectationForMockedWorker([](AsyncWorkerMock& worker){
		EXPECT_CALL(worker, doJob(_, _)).Times(0);
	});
	for (auto i : {0, 2})
		EXPECT_CALL(*(static_cast<AsyncWorkerMock*>(workers_[i].get())), 
			doJob(_, _)).Times(jobs);
	EXPECT_CALL(*(static_cast<AsyncWorkerMock*>(workers_[1].get())), 
			doJob(_, _)).Times(1);

	runAsyncJobsAndWaitForFinish(jobs, "collect", getExponentOf, "strandA");
	runAsyncJobsAndWaitForFinish(1, "collect", getExponentOf);
	runAsyncJobsAndWaitForFinish(jobs, "collect", getExponentOf, "strandB");
}