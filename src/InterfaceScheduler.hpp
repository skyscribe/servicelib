#pragma once 

#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

struct ParaArgsBase{};

template <class ... Args>
struct ParamArgs : public ParaArgsBase{
	ParamArgs(const Args&... args) : parameters(args...){}
	std::tuple<Args...> parameters;
};

template <std::size_t I, class ... Types>
typename std::tuple_element<I, std::tuple<Types...> >::type const& get(const ParamArgs<Types ...>& args){
	return std::get<I>(args.parameters);
}

typedef std::function<bool(const ParaArgsBase&)> CallbackType;
typedef std::function<bool()> Callable;
class AsyncWorker;
typedef std::shared_ptr<AsyncWorker> AsyncWorkerPtr;
class SyncWorker;
const size_t defaultPoolSize = 4;

struct CallProperty{
	bool async; //will the job be scheduled asynchronously (under same context)
	bool waitForDone; //if interfaceCall will be blocked (for done) or not, will be ignored for async call
	Callable onCallDone;
	std::string strand; //calls with same strand will be scheduled by same thread, async only
};

typedef std::vector<AsyncWorkerPtr> AsyncWorkerQueue;
class InterfaceScheduler{
public:
	InterfaceScheduler(AsyncWorkerQueue queue = AsyncWorkerQueue(), std::shared_ptr<SyncWorker> sync = nullptr)
		: asyncWorkers_(queue), syncWorker_(sync), started_(0){};

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

	//Actual call under the hood - Use above wrapper overload as possible!
	template <class ... Args>
	bool interfaceCall(const std::string& idStr, bool async = false, bool waitForDone = true,
			Callable&& onCallDone = Callable(), const std::string& strand = "", const Args& ... args){
		mappingLock_.lock();
		auto actIt = actionMapping_.find(idStr);
		if (actIt == actionMapping_.end()){
			std::cout << "Non-existent interface to call:" << idStr << std::endl;
			mappingLock_.unlock();
			return false;
		}else{
			auto action = actIt->second; //copy explicitly to ensure thread-safety
			mappingLock_.unlock();

			return invokeCall([args..., action]() -> bool{
				ParamArgs<Args ...> param(args...);
				return action(param);
			}, async, waitForDone, strand, std::forward<Callable>(onCallDone));
		}
	}

	void dumpWorkersLoad(std::ostream& collector)const;
	void getStatistics(size_t& asyncWorksCnt, size_t& totalLoad);

private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;
	const AsyncWorkerPtr& getWorkerForSchedule(const std::string& strand);
	const AsyncWorkerPtr& getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle);

	void createWorksIfNotInitialized(size_t poolSize);

	//Invoke the actual call wrapped in cb/onDone
	bool invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& strand, Callable&& onDone);

	std::unordered_map<std::string, CallbackType> actionMapping_;
	std::mutex mappingLock_;

	std::vector<AsyncWorkerPtr> asyncWorkers_;
	std::shared_ptr<SyncWorker> syncWorker_;
	std::atomic<bool>			started_;

	std::unordered_map<std::string, AsyncWorkerPtr> strands_;
};