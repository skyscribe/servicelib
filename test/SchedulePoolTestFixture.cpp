#include "SchedulePoolTestFixture.h"
#include "Helper.hpp"
#include <chrono>

using namespace std;
AsyncWorkerQueue createMockedWorkers(size_t num){
	AsyncWorkerQueue queue;
	for (auto i = 0; i < num; ++i)
		queue.push_back(make_shared<AsyncWorkerMock>(true/*do real jobs*/));
	return queue;
}

void SchedulePoolTest::SetUp(){
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

pair<size_t, size_t> SchedulePoolTest::runAsyncJobsAndWaitForFinish(size_t jobCnt, const string& jobName, 
		function<int(int)>&& getParam, function<void()>&& onJobDone, const string& desc, const string& strand){
	atomic<int> done(0);
	auto callTm = scheduleAllJobs(done, jobCnt, jobName, forward<function<int(int)>>(getParam),
		forward<function<void()>>(onJobDone), desc, strand);
	auto waitTm = waitForAllJobsDone(done, jobCnt, desc);		
	EXPECT_EQ(done, jobCnt);
	return {callTm, waitTm};
}

size_t SchedulePoolTest::scheduleAllJobs(atomic<int>& done, size_t jobCnt, const string& jobName, 
		function<int(int)>&& getParam, function<void()>&& onJobDone, const string& desc,
		const string& strand){
	return profileFor([&]{
		for (int i = 0; i < jobCnt; ++i)
			sched_.interfaceCall(jobName, true, false, [&]()->bool{
					if (onJobDone)
						onJobDone();
					done++;
				}, strand, getParam(i));
	}, desc, false /*don't print*/);		
}

size_t SchedulePoolTest::waitForAllJobsDone(atomic<int>& done, size_t jobCnt, const string& desc){
	return profileFor([&]{
			for(auto i = 0; (i < 1000) && (done != jobCnt); ++i)
				this_thread::sleep_for(chrono::milliseconds(timeForSingleJob));
		}, desc + "-join", false/*dont print*/);
}