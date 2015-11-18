#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <functional>
#include "gtest/gtest.h"
#include "WorkerMock.hpp"
#include "InterfaceScheduler.hpp"

typedef std::vector<std::shared_ptr<AsyncWorkerMock>> WorkerList;
AsyncWorkerQueue createMockedWorkers(size_t num);
using std::function;

class SchedulePoolTest : public ::testing::Test{
protected:
	size_t poolSize_;
	AsyncWorkerQueue workers_;

	InterfaceScheduler sched_;
	std::atomic<int> sharedVar_;
	const size_t timeForSingleJob = 5;

	std::mutex lock_;
	std::vector<std::pair<int, std::thread::id>> threadMapping_;

	SchedulePoolTest() : ::testing::Test(), poolSize_(4), workers_(createMockedWorkers(poolSize_)),
			sched_(workers_), sharedVar_(0){}

	virtual void SetUp() override;

	void setExpectationForMockedWorker(std::function<void(AsyncWorkerMock&)> action){
		for(auto worker : workers_)
			action(*(static_cast<AsyncWorkerMock*>(worker.get())));
	}

	virtual void TearDown() override{
		sched_.stop();
	}

	std::pair<size_t, size_t> runAsyncJobsAndWaitForFinish(size_t jobCnt, const std::string& jobName, 
			function<int(int)>&& getParam, const std::string& strand = "", 
			std::function<void()>&& onJobDone = std::function<void()>(),
			const std::string& desc = "");
private:
	size_t scheduleAllJobs(std::atomic<int>& done, size_t jobCnt, const std::string& jobName, 
			function<int(int)>&& getParam, std::function<void()>&& onJobDone, const std::string& desc = "",
			const std::string& strand = "");

	size_t waitForAllJobsDone(std::atomic<int>& done, size_t jobCnt, const std::string& desc);
};