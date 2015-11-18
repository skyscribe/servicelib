#include "Worker.h"
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
	auto action = calls_.begin()->first;
	auto onDone = calls_.begin()->second;
	calls_.erase(calls_.begin());

	//release lock during execution so other jobs can be accepted without blocking
	queueLock_.unlock();
	runTheCall(std::move(action), std::move(onDone));
	queueLock_.lock();
	outstandingJobs_--;
}

void runTheCall(Callable action, Callable onDone){
	assert(action);
	action();
	if (onDone)
		onDone();
}

bool AsyncWorker::doJob(Callable call, Callable onDone){
	{
		std::unique_lock<std::mutex> lock(queueLock_);
		calls_.push_back({call, onDone});
	}
	queueCond_.notify_all();
	outstandingJobs_++;
	return true;
}

bool AsyncWorker::doSyncJob(Callable call, Callable onDone){
	atomic<bool> finished(false);
	doJob(std::forward<Callable>(call), [&]() -> bool{
		if (onDone)
			onDone();
		finished = true;
		//cout << "sync job done!" << endl;
		return true;
	});
	while(!finished)
		this_thread::sleep_for(chrono::milliseconds(10));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		queueLock_.lock();
	}	
}