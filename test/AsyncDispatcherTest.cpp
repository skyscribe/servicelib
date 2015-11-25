#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>
#include <iostream>

#include "AsyncDispatcher.h"
#include "WorkerMock.hpp"

using namespace std;

class AsyncDispatcherTest : public ::testing::Test{
protected:
	AsyncWorkerQueue asyncs_;
	AsyncDispatcher dispatcher_;

	AsyncDispatcherTest(size_t workers = 1) : Test(), asyncs_(createWorkers(workers)),
			dispatcher_(asyncs_), workerCnt_(workers){}

	AsyncWorkerMock& getWorker(size_t i = 0){
		return dynamic_cast<AsyncWorkerMock&>(*(asyncs_[i]));
	}

	virtual void SetUp() override{
		EXPECT_CALL(getWorker(), blockUntilReady()).Times(1);
		EXPECT_CALL(getWorker(), stop()).Times(::testing::AnyNumber());
		dispatcher_.start(1);
	}

	virtual void TearDown() override{
		dispatcher_.stop();
	}

private:
	AsyncWorkerQueue createWorkers(size_t cnt){
		AsyncWorkerQueue queue;
		for (auto i = 0; i < cnt; ++i)
			queue.push_back(make_shared<AsyncWorkerMock>(true));
		return queue;
	}
	size_t workerCnt_;
};

using namespace ::testing;
const std::string serviceName = "for.testing";
TEST_F(AsyncDispatcherTest, cancelPendingJobs_allOfThemCancelled){
	EXPECT_CALL(getWorker(), doJob(serviceName, _, _)).Times(3);
	EXPECT_CALL(getWorker(), cancelJobsFor(serviceName)).Times(1);

	//Using atomics to synchronize such that job state is under exact control
	auto doNothing = []() -> bool{return true;};
	atomic<bool> jobDoneFlag(false);
	atomic<bool> jobStarted(false);
	atomic<int> calledCounter(0);

	//mimic the long-lasting job to block the queue
	auto syncJob = [&]()->bool{
		jobStarted = true;
		calledCounter++;
		while(!jobDoneFlag)
			std::this_thread::yield();
	};
	// all jobs in a strand in worker 1
	for (auto i : {1,2,3})
		dispatcher_.scheduleJob(serviceName, doNothing, syncJob, false, "test");

	//synchronize until first job started (actually it will block others)
	while(!jobStarted)
		std::this_thread::yield();
	dispatcher_.cancelJobsFor(serviceName);
	jobDoneFlag = true;

	//Need to stop explicitly to avoid sync atomic variables out of scope
	dispatcher_.stop();
	EXPECT_EQ(calledCounter, 1);
}