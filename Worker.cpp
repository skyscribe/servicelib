#include "Worker.h"

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

		for (auto& actions : calls_){
			actions.first();
			if (actions.second)
				actions.second();
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
}