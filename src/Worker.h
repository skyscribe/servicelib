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
	AsyncWorker() : thread_(std::bind(&AsyncWorker::run, this)), active_(1), ready_(0){}
	~AsyncWorker() {stop();}

	void blockUntilReady();
	void stop();
	bool doJob(Callable&& call, Callable&& onDone);
	bool doSyncJob(Callable&& call, Callable&& onDone);
	void run();
	
private:
	void scheduleFirstOutstandingJob();
	std::vector<std::pair<Callable, Callable>> calls_;

	std::atomic<bool> ready_;
	std::condition_variable queueCond_;
	std::mutex queueLock_;
	std::atomic<bool> active_;
	std::thread thread_;
};