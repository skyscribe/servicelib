#include "Worker.h"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>
using namespace std;

bool SyncWorker::doJob(Callable call, Callable onDone){
	auto ret = call();
	if (onDone){
		onDone();
	}
	return ret;
}

void AsyncWorker::blockUntilReady(){
	while(!ready_)
		this_thread::sleep_for(chrono::milliseconds(1));
}

void AsyncWorker::run(){
	ready_ = 1;
	while(active_){
		std::unique_lock<std::mutex> lock(queueLock_);
		auto shouldSchedule = [&]() -> bool {
			return (!calls_.empty()) || (!active_);			
		};
		queueCond_.wait(lock, shouldSchedule);

		while(!calls_.empty())
			scheduleFirstOutstandingJob();
	};
}

void runTheCall(Callable, Callable);
void AsyncWorker::scheduleFirstOutstandingJob(){
	//explicitly copy out to avoid iterator changed asynchronously
	auto ctx = *calls_.begin();
	calls_.erase(calls_.begin());
	outstandingJobs_--;

	//release lock during execution so other jobs can be accepted without blocking
	queueLock_.unlock();
	runTheCall(std::move(ctx.action), std::move(ctx.onDone));
	queueLock_.lock();
}

void runTheCall(Callable action, Callable onDone){
	assert(action);
	action();
	if (onDone)
		onDone();
}

bool AsyncWorker::doJob(const std::string& name, Callable call, Callable onDone){
	{
		std::lock_guard<std::mutex> lock(queueLock_);
		calls_.push_back({call, onDone, name});
	}
	queueCond_.notify_all();
	outstandingJobs_++;
	return true;
}

bool AsyncWorker::doSyncJob(const std::string& name, Callable call, Callable onDone){
	bool finished = false;
	mutex flagMutex;
	condition_variable cond;
	
	doJob(name, std::forward<Callable>(call), [&]() -> bool{
		if (onDone)
			onDone();
		std::lock_guard<mutex> lock(flagMutex);
		finished = true;
		cond.notify_all();
		return true;
	});

	unique_lock<mutex> lock(flagMutex);
	cond.wait(lock, [&]{return finished;});
	return true;
}

void AsyncWorker::stop() {
	if (!active_)
		return;

	queueLock_.lock();
	waitForOutstandingJobsToFinish();
	active_ = false;
	queueCond_.notify_all();
	queueLock_.unlock();

	thread_.join();
}

void AsyncWorker::waitForOutstandingJobsToFinish(){
	while (!calls_.empty()){
		queueLock_.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		queueLock_.lock();
	}	
}

void AsyncWorker::cancelJobsFor(const std::string& name){
	std::lock_guard<std::mutex> lock(queueLock_);
	calls_.erase(std::remove_if(calls_.begin(), calls_.end(), [&](const JobContext& it){
		return it.id == name;
	}), calls_.end());
	outstandingJobs_ = calls_.size();
}