#include "InterfaceScheduler.hpp"
#include "Worker.h"
#include <iostream>
#include <algorithm>
using namespace std;

InterfaceScheduler::InterfaceScheduler(){}

void InterfaceScheduler::start(size_t poolSize_){
	syncWorker_.reset(new SyncWorker());
	for (auto i = 0; i < poolSize_; ++i)
		asyncWorkers_.push_back(make_shared<AsyncWorker>());
	for (auto worker : asyncWorkers_)
		worker->blockUntilReady();
}

void InterfaceScheduler::stop(){
	for(auto worker : asyncWorkers_)
		worker->stop();
}

void InterfaceScheduler::registerInterface(const std::string& idStr, CallbackType action){
	actionMapping_[idStr] = action;
}

void InterfaceScheduler::unRegiterInterface(const std::string& idStr){
	if (actionMapping_.find(idStr) != actionMapping_.end())
		actionMapping_.erase(idStr);
	else
		std::cout << "Non-existent interface type:" << idStr << std::endl;
}

bool InterfaceScheduler::invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& strand, Callable&& onDone){
	if (!async)
		return syncWorker_->doJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
	else
		if (!waitForDone)
			return getWorkerForSchedule(strand)->doJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
		else
			return getWorkerForSchedule(strand)->doSyncJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
}

const AsyncWorkerPtr& InterfaceScheduler::getWorkerForSchedule(const std::string& strand){
	auto it = min_element(asyncWorkers_.begin(), asyncWorkers_.end(), [](const AsyncWorkerPtr& a, const AsyncWorkerPtr& b) -> bool{
		return a->getLoad() < b->getLoad();
	});
	
	if (strand.empty())
		return *it;
	else
		return getStrandWorkerForSchedule(strand, *it);

	//for (auto worker : asyncWorkers_)
	//	cout << "Worker<" << worker->getId() << ",load=" << worker->getLoad() << endl;
	//cout << "selected:" << (*it)->getId() << ",load=" << (*it)->getLoad() << endl;
	return *it;
}

const AsyncWorkerPtr& InterfaceScheduler::getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle){
	if (strands_.find(strand) == strands_.end())
		strands_[strand] = idle;
	return strands_[strand];
}