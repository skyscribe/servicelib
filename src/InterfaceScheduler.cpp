#include "InterfaceScheduler.hpp"
#include "Worker.h"
#include <iostream>
#include <cassert>
#include <algorithm>
using namespace std;

void InterfaceScheduler::start(size_t poolSize){
	if (started_)
		return;

	createWorkersIfNotInitialized(poolSize);
	for (auto worker : asyncWorkers_)
		worker->blockUntilReady();
	started_ = true;
}

void InterfaceScheduler::createWorkersIfNotInitialized(size_t poolSize){
	if (!syncWorker_)
		syncWorker_.reset(new SyncWorker());
	if (asyncWorkers_.empty())
		for (auto i = 0; i < poolSize; ++i)
			asyncWorkers_.push_back(make_shared<AsyncWorker>());
	else
		assert(asyncWorkers_.size() == poolSize);	
}


void InterfaceScheduler::stop(){
	for(auto worker : asyncWorkers_)
		worker->stop();
}

void InterfaceScheduler::registerInterface(const std::string& idStr, CallbackType action, std::string typeId){
	if (!action)
		throw std::invalid_argument("Empty function provided as callback, name=" + idStr);
	lock_guard<mutex> guard(mappingLock_);
	actionMapping_[idStr] = {action, typeId};
}

void InterfaceScheduler::unRegiterInterface(const std::string& idStr){
	lock_guard<mutex> guard(mappingLock_);	
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
	//dumpWorkersLoad(cout);
	if (strand.empty())
		return *it;
	else
		return getStrandWorkerForSchedule(strand, *it);
}

const AsyncWorkerPtr& InterfaceScheduler::getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle){
	std::lock_guard<mutex> guard(strandsLock_);
	if (strands_.find(strand) == strands_.end())
		strands_[strand] = idle;
	return strands_[strand];
}

void InterfaceScheduler::dumpWorkersLoad(std::ostream& collector)const{
	collector << "AsyncWorkers load info: total=" << asyncWorkers_.size() << endl;
	for (auto worker : asyncWorkers_)
		collector << "\tWorker<" << worker->getId() << ",load=" << worker->getLoad() << endl;	
}

void InterfaceScheduler::getStatistics(size_t& asyncWorksCnt, size_t& totalLoad){
	asyncWorksCnt = asyncWorkers_.size();
	totalLoad = 0;
	for (auto worker : asyncWorkers_)
		totalLoad += worker->getLoad();
}

InterfaceScheduler::ActionStore InterfaceScheduler::fetchStoredCallbackByServiceId(const std::string& idStr){
	std::lock_guard<std::mutex> guard(mappingLock_);
	auto actIt = actionMapping_.find(idStr);
	if (actIt == actionMapping_.end())
		return {CallbackType(), ""};
	else
		return actIt->second; 
}