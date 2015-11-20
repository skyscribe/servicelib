#pragma once 
#include "ArgDefs.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>

class InterfaceScheduler{
public:
	InterfaceScheduler(AsyncWorkerQueue queue = AsyncWorkerQueue(), std::shared_ptr<SyncWorker> sync = nullptr)
		: asyncWorkers_(queue), syncWorker_(sync), started_(0){};

	static const size_t defaultPoolSize = 4;
	void start(size_t asyncPoolSize = defaultPoolSize);
	void stop();

	void registerInterface(const std::string& idStr, CallbackType action);
	void unRegiterInterface(const std::string& idStr);

	//Schedule a previously registered interface cally by CallProperty (see its definition)
	template <class ... Args>
	bool interfaceCall(const std::string& idStr, CallProperty&& prop, const Args& ...args){
		return interfaceCall<Args ...>(idStr, prop.async, prop.waitForDone, std::forward<Callable>(prop.onCallDone),
			prop.strand, args...);
	}

	void dumpWorkersLoad(std::ostream& collector)const;
	void getStatistics(size_t& asyncWorksCnt, size_t& totalLoad);

private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;
	const AsyncWorkerPtr& getWorkerForSchedule(const std::string& strand);
	const AsyncWorkerPtr& getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle);

	//Actual call under the hood
	template <class ... Args>
	bool interfaceCall(const std::string& idStr, bool async, bool waitForDone, Callable&& onCallDone,
		const std::string& strand, const Args& ... args){
		auto action = fetchStoredCallbackByServiceId(idStr);
		if (!action)
			return false;
		else
			return invokeCall([args..., action]() -> bool{
				ParamArgs<Args ...> param(args...);
				return action(param);
			}, async, waitForDone, strand, std::forward<Callable>(onCallDone));			
	}

	void createWorkersIfNotInitialized(size_t poolSize);
	CallbackType fetchStoredCallbackByServiceId(const std::string& idStr);

	//Invoke the actual call wrapped in cb/onDone
	bool invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& strand, Callable&& onDone);

	std::unordered_map<std::string, CallbackType> actionMapping_;
	mutable std::mutex mappingLock_;

	std::vector<AsyncWorkerPtr> asyncWorkers_;
	std::shared_ptr<SyncWorker> syncWorker_;
	std::atomic<bool>			started_;

	std::unordered_map<std::string, AsyncWorkerPtr> strands_;
	mutable std::mutex strandsLock_;
};