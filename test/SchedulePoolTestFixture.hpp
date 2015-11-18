#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <functional>
#include "gtest/gtest.h"

typedef std::vector<shared_ptr<AsyncWorkerMock>> WorkerList;
AsyncWorkerQueue createMockedWorkers(size_t num){
	AsyncWorkerQueue queue;
	for (auto i = 0; i < num; ++i)
		queue.push_back(make_shared<AsyncWorkerMock>(true/*do real jobs*/));
	return queue;
}


class SchedulePoolTest : public ::testing::Test{
protected:
	size_t poolSize_;
	AsyncWorkerQueue workers_;

	InterfaceScheduler sched_;
	atomic<int> sharedVar_;
	const size_t timeForSingleJob = 5;

	std::mutex lock_;
	std::vector<std::pair<int, thread::id>> threadMapping_;

	SchedulePoolTest() : ::testing::Test(), poolSize_(4), workers_(createMockedWorkers(poolSize_)),
			sched_(workers_), sharedVar_(0){}

	virtual void SetUp() override{
		setExpectationForMockedWorker([&](AsyncWorkerMock& worker){
			EXPECT_CALL(worker, blockUntilReady()).Times(1);
			EXPECT_CALL(worker, stop()).Times(1);
			EXPECT_CALL(worker, getLoad()).Times(::testing::AnyNumber());
			EXPECT_CALL(worker, getId()).Times(::testing::AnyNumber());
		});

		sched_.registerInterface("sum", [&](const ParaArgsBase& p) -> bool {
			//this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
			sharedVar_ += get<0>(static_cast<const ParamArgs<int>&>(p));
			return true;
		});

		registerInterfaceFor<int>(sched_, "collect", [&](const ParamArgs<int> p)->bool{
			lock_.lock();
			threadMapping_.push_back({get<0>(p), this_thread::get_id()});
			lock_.unlock();
		});

		profileFor([&]{
			sched_.start(poolSize_);
		}, "start scheduler");
	}

	void setExpectationForMockedWorker(function<void(AsyncWorkerMock&)> action){
		for(auto worker : workers_)
			action(*(static_cast<AsyncWorkerMock*>(worker.get())));
	}

	virtual void TearDown() override{
		sched_.stop();
	}

	std::pair<size_t, size_t> runAsyncJobsAndWaitForFinish(size_t jobCnt, const std::string& jobName, 
			function<int(int)>&& getParam, function<void()>&& onJobDone = function<void()>(),
			const std::string& desc = "", const std::string& strand = ""){
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