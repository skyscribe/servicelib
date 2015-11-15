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

	std::pair<size_t, size_t> runAsyncJobsAndWaitForFinish(size_t jobCnt, const std::string& jobName, 
			function<int(int)>&& getParam, function<void()>&& onJobDone, const std::string& desc = "",
			const std::string& strand = ""){
		atomic<int> done(0);
		auto callTm = scheduleAllJobs(done, jobCnt, jobName, forward<function<int(int)>>(getParam),
			forward<function<void()>>(onJobDone), desc, strand);
		auto waitTm = waitForAllJobsDone(done, jobCnt, desc);		
		EXPECT_EQ(done, jobCnt);
		return {callTm, waitTm};
	}

	size_t scheduleAllJobs(atomic<int>& done, size_t jobCnt, const std::string& jobName, 
			function<int(int)>&& getParam, function<void()>&& onJobDone, const std::string& desc = "",
			const std::string& strand = ""){
		return profileFor([&]{
			for (int i = 0; i < jobCnt; ++i)
				sched_.interfaceCall(jobName, true, false, [&]()->bool{
						if (onJobDone)
							onJobDone();
						done++;
					}, strand, getParam(i));
		}, desc, false /*don't print*/);		
	}

	size_t waitForAllJobsDone(atomic<int>& done, size_t jobCnt, const std::string& desc){
		return profileFor([&]{
				for(auto i = 0; (i < 1000) && (done != jobCnt); ++i)
					this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
			}, desc + "-join", false/*dont print*/);
	}
};


TEST_F(SchedulePoolTest, schedulePoolSizeJobs_jobsDistributedEvenly){
	const size_t jobs = poolSize_;
	mutex lock;
	set<thread::id> idsList;
	auto tmInfo = runAsyncJobsAndWaitForFinish(jobs, "heavy", [](int i) -> int{
			return 1 << i;
		}, [&](){
			lock.lock();
			idsList.insert(this_thread::get_id());
			lock.unlock();		
		}
	);

	//NOTE: check on running time may not be stable and subject to CPU scheduling!
	ASSERT_LT(tmInfo.first, timeForSingleJob);
	EXPECT_LT(tmInfo.first + tmInfo.second, timeForSingleJob*jobs);
	EXPECT_EQ(idsList.size(), jobs);
	EXPECT_EQ(sharedVar_, (1 << jobs) - 1);	
}

TEST_F(SchedulePoolTest, scheduleWithAffinity_allJobsRunAsStrand){
	atomic<int> done(0);
	const size_t jobs = poolSize_*2;

	auto tmInfo = runAsyncJobsAndWaitForFinish(jobs, "light", [&](int i) -> int{
			return 1 << i;
		}, function<void()>(), "", "strand");

	EXPECT_EQ(threadMapping_.size(), jobs);
	std::set<thread::id> idSets;
	for (auto item : threadMapping_)
		idSets.insert(item.second);
	EXPECT_EQ(idSets.size(), 1);
}