#include "Worker.h"
#include <iostream>
using namespace std;

bool SyncWorker::doJob(Callable&& call, Callable&& onDone){
	auto ret = call();
	if (onDone){
		onDone();
	}
	return ret;
}

void AsyncWorker::run(){
	while(runFlag_){
		std::unique_lock<std::mutex> lock(callLock_);
		cond_.wait(lock, [&](){
			return !(calls_.empty());
		});

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
	runFlag_ = 0;
	thread_.join();
	cout << "thread stopped now!" << "Jobs:" << calls_.size() << endl;
}