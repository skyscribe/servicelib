#include "InterfaceScheduler.hpp"
#include "AsyncDispatcher.h"
#include "Worker.h"
#include <iostream>
#include <cassert>
#include <algorithm>
using namespace std;

InterfaceScheduler::InterfaceScheduler(AsyncWorkerQueue queue, std::shared_ptr<SyncWorker> sync)
		: asyncDispather_(make_shared<AsyncDispatcher>(queue)), syncWorker_(sync), started_(0){};

void InterfaceScheduler::start(size_t poolSize){
	if (started_)
		return;

	if (!syncWorker_)
		syncWorker_.reset(new SyncWorker());
	asyncDispather_->start(poolSize);

	started_ = true;
}

void InterfaceScheduler::stop(){
	asyncDispather_->stop();
}

void InterfaceScheduler::subscribeForRegistration(const std::string& idStr, std::function<void()>&& cb){
	lock_guard<recursive_mutex> guard(mappingLock_);
	if (actionMapping_.find(idStr) != actionMapping_.end())
		cb();
	else
		notifyMapping_[idStr].push_back(cb);
}

void InterfaceScheduler::registerInterface(const std::string& idStr, CallbackType action, std::string typeId){
	if (!action)
		throw std::invalid_argument("Empty function provided as callback, name=" + idStr);
	
	lock_guard<recursive_mutex> guard(mappingLock_);
	if (actionMapping_.find(idStr) != actionMapping_.end())
		throw std::logic_error("Duplicated registration for " + idStr + " - unregister firstly");
	
	actionMapping_[idStr] = {action, typeId};
	notifySubscribersOnRegistration(idStr);
}

void InterfaceScheduler::notifySubscribersOnRegistration(const std::string& idStr){
	if (notifyMapping_.find(idStr) != notifyMapping_.end()){
		for (auto cb : notifyMapping_[idStr])
			cb();
		notifyMapping_.erase(idStr);
	}	
}

void InterfaceScheduler::unRegiterInterface(const std::string& idStr){
	lock_guard<recursive_mutex> guard(mappingLock_);	
	if (actionMapping_.find(idStr) != actionMapping_.end())
		actionMapping_.erase(idStr);
	else
		std::cout << "Non-existent interface type:" << idStr << std::endl;
}

bool InterfaceScheduler::isServiceRegistered(const std::string& idStr)const{
	lock_guard<recursive_mutex> guard(mappingLock_);
	return actionMapping_.find(idStr) != actionMapping_.end();
}

bool InterfaceScheduler::invokeCall(Callable&& cb, bool async, bool waitForDone, 
		const std::string& strand, Callable&& onDone){
	if (!async)
		return syncWorker_->doJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
	else
		return asyncDispather_->scheduleJob(waitForDone, strand, std::forward<Callable>(cb),
			std::forward<Callable>(onDone));
}

bool InterfaceScheduler::isCallRegisteredAndTypesMatch(const std::string& idStr,
		const std::string&& callType, CallbackType& action){
	std::string storedType;
	if(!fetchStoredCallbackByServiceId(idStr, action, storedType))
		return false;

	if ((callType != storedType))
		throw std::invalid_argument("Expected type: <" + storedType + ">, actual:" + callType);
	return true;
}

bool InterfaceScheduler::fetchStoredCallbackByServiceId(const std::string& idStr,
		CallbackType& call, std::string& typeStr){
	std::lock_guard<recursive_mutex> guard(mappingLock_);
	const auto& actIt = actionMapping_.find(idStr);
	if (actIt == actionMapping_.end())
		return false;

	std::tie(call, typeStr) = actIt->second;
	return true;
}