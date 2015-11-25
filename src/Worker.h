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
	virtual bool doJob(Callable call, Callable onDone);
};

class AsyncWorker{
public:
	AsyncWorker() : thread_(std::bind(&AsyncWorker::run, this)),
		active_(1), ready_(0), outstandingJobs_(0){}
	virtual ~AsyncWorker() {stop();}

	virtual void blockUntilReady();
	virtual void stop();
	virtual bool doJob(const std::string& name, Callable call, Callable onDone);
	virtual bool doSyncJob(const std::string& name, Callable call, Callable onDone);
	virtual void cancelJobsFor(const std::string& name);

	virtual size_t getLoad() {return outstandingJobs_;}
	virtual std::thread::id getId() {return thread_.get_id();}

private:
	void run();
	void scheduleFirstOutstandingJob();
	void waitForOutstandingJobsToFinish();
	std::vector<std::pair<Callable, Callable>> calls_;

	std::atomic<bool> ready_;
	std::condition_variable queueCond_;
	std::mutex queueLock_;
	std::atomic<bool> active_;
	std::thread thread_;
	//keep track of undone jobs, may not equal to calls_.size()!
	std::atomic<size_t> outstandingJobs_; 
};