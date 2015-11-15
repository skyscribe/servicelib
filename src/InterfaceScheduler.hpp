#pragma once 

#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

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
	bool async;
	bool waitForDone;
	Callable onCallDone;
	std::string strand;
};

class InterfaceScheduler{
public:
	InterfaceScheduler();

	void start(size_t asyncPoolSize = defaultPoolSize);
	void stop();

	void registerInterface(const std::string& idStr, CallbackType action);
	void unRegiterInterface(const std::string& idStr);

	//Schedule a previously registered interface cally by asynchrously or synchronusly, wait for execution
	// done (waitForDone) or exit after job scheduled, and attach an action after job actually invoked
	// If a strand is specified, all calls within same strand will be executed sequentially (one after another) 
	template <class ... Args>
	bool interfaceCall(const std::string& idStr, bool async = false, bool waitForDone = true,
			Callable&& onCallDone = Callable(), const std::string& strand = "", const Args& ... args){
		auto actIt = actionMapping_.find(idStr);
		if (actIt == actionMapping_.end()){
			std::cout << "Non-existent interface to call:" << idStr << std::endl;
			return false;
		}else{
			return invokeCall([args..., actIt]() -> bool{
				ParamArgs<Args ...> param(args...);
				return actIt->second(param);
			}, async, waitForDone, strand, std::forward<Callable>(onCallDone));
		}
	}

	template <class ... Args>
	bool interfaceCall(const std::string& idStr, CallProperty&& prop, const Args& ...args){
		return interfaceCall<Args ...>(idStr, prop.async, prop.waitForDone, std::forward<Callable>(prop.onCallDone),
			prop.strand, args...);
	}

	void dumpWorkersLoad(std::ostream& collector)const;
private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;
	const AsyncWorkerPtr& getWorkerForSchedule(const std::string& strand);
	const AsyncWorkerPtr& getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle);

	//Invoke the actual call wrapped in cb/onDone
	bool invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& strand, Callable&& onDone);

	std::unordered_map<std::string, CallbackType> actionMapping_;
	std::vector<AsyncWorkerPtr> asyncWorkers_;
	std::shared_ptr<SyncWorker> syncWorker_;

	std::unordered_map<std::string, AsyncWorkerPtr> strands_;
};