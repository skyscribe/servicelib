#include "InterfaceScheduler.hpp"
#include "Worker.h"

#include <iostream>

InterfaceScheduler::InterfaceScheduler(){}

void InterfaceScheduler::start(){
	asyncWorker_.reset(new AsyncWorker());
	syncWorker_.reset(new SyncWorker());
	asyncWorker_->blockUntilReady();
}

void InterfaceScheduler::stop(){
	asyncWorker_->stop();
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

bool InterfaceScheduler::invokeCall(Callable&& cb, bool async, bool waitForDone, Callable&& onDone){
	if (!async)
		return syncWorker_->doJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
	else
		if (!waitForDone)
			return asyncWorker_->doJob(std::forward<Callable>(cb), std::forward<Callable>(onDone));
		else
			return asyncWorker_->doSyncJob(std::forward<Callable>(cb));
}
