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
	std::unique_lock<std::mutex> lock(startLock_);
	condReady_.wait(lock, [&]() -> bool{
		return ready_;
	});
}

void AsyncWorker::run(){
	ready_ = 1;
	condReady_.notify_all();

	while(runFlag_){
		std::unique_lock<std::mutex> lock(callLock_);
		//cout << "#cond wait for jobs!" << endl;
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
		//cout << "#Job handled!" << endl;
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

bool AsyncWorker::doSyncJob(Callable&& call){
	bool finished = false;
	std::condition_variable cv;

	doJob(std::forward<Callable>(call), [&]() -> bool{
		finished = true;
		cv.notify_all();
		return true;
	});

	std::mutex mut;
	std::unique_lock<std::mutex> lock(mut);
	cv.wait(lock, [&]{return finished;});
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
	cout << "joining thread!" << endl;

	thread_.join();
	cout << "thread stopped now!" << "Jobs:" << calls_.size() << endl;
}