#pragma once

#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

typedef std::function<bool()> Callable;
class SyncWorker{
public:
	bool doJob(Callable&& call, Callable&& onDone);
};

class AsyncWorker{
public:
	AsyncWorker() : thread_(std::bind(&AsyncWorker::run, this)),
		active_(1), ready_(0), outstandingJobs_(0){}
	~AsyncWorker() {stop();}

	void blockUntilReady();
	void stop();
	bool doJob(Callable&& call, Callable&& onDone);
	bool doSyncJob(Callable&& call, Callable&& onDone);
	void run();
	size_t getLoad() {return outstandingJobs_;}
	std::thread::id getId() {return thread_.get_id();}

private:
	void scheduleFirstOutstandingJob();
	void waitForOutstandingJobsToFinish();
	std::vector<std::pair<Callable, Callable>> calls_;

	std::atomic<bool> ready_;
	std::condition_variable queueCond_;
	std::mutex queueLock_;
	std::atomic<bool> active_;
	std::thread thread_;

	std::atomic<size_t> outstandingJobs_; //keep track of undone jobs, may not equal to calls_.size()!
};