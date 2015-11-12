#include "Worker.h"
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

bool SyncWorker::doJob(Callable&& call, Callable&& onDone){
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
	while(runFlag_){
		std::unique_lock<std::mutex> lock(callLock_);
		cond_.wait(lock, [&](){
			return (!calls_.empty()) || (!runFlag_);
		});

		//cout << "#run jobs " << endl;
		while (!calls_.empty()){
			auto it = calls_.begin();
			it->first();
			if (it->second)
				it->second();
			calls_.erase(it);
		}
	};
}

bool AsyncWorker::doJob(Callable&& call, Callable&& onDone){
	{
		std::unique_lock<std::mutex> lock(callLock_);
		calls_.push_back({call, onDone});
	}
	cond_.notify_all();
	return true;
}

bool AsyncWorker::doSyncJob(Callable&& call, Callable&& onDone){
	atomic<bool> finished(false);
	doJob(std::forward<Callable>(call), [&]() -> bool{
		onDone();
		finished = true;
		return true;
	});

	while(!finished)
		this_thread::sleep_for(chrono::milliseconds(10));
	return true;
}

void AsyncWorker::stop() {
	callLock_.lock();
	while (!calls_.empty()){
		callLock_.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		callLock_.lock();
	}
	runFlag_ = 0;
	cond_.notify_all();
	callLock_.unlock();
	thread_.join();
	//cout << "thread stopped now!" << "Jobs:" << calls_.size() << endl;
}