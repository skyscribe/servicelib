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

class InterfaceScheduler{
public:
	InterfaceScheduler();

	void start(size_t asyncPoolSize = defaultPoolSize);
	void stop();

	template <class ... Args, class ActionType>
	void registerInterfaceFor(const std::string& idStr, ActionType action){
		registerInterface(idStr, [=](const ParaArgsBase& p) -> bool{
			typedef ParamArgs<Args ...> ActualType;
			return action(static_cast<const ActualType&>(p));
		});
	}

	void registerInterface(const std::string& idStr, CallbackType action);
	void unRegiterInterface(const std::string& idStr);

	template <class ... Args>
	bool interfaceCall(const std::string& idStr, bool async = false, bool waitForDone = true,
			Callable&& onCallDone = Callable(), const Args& ... args){
		auto actIt = actionMapping_.find(idStr);
		if (actIt == actionMapping_.end()){
			std::cout << "Non-existent interface to call:" << idStr << std::endl;
			return false;
		}else{
			return invokeCall([args..., actIt]() -> bool{
				ParamArgs<Args ...> param(args...);
				return actIt->second(param);
			}, async, waitForDone, std::forward<Callable>(onCallDone));
		}
	}

private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;
	const AsyncWorkerPtr& getIdleWorker()const;

	bool invokeCall(Callable&& cb, bool async, bool waitForDone, Callable&& onDone);
	std::unordered_map<std::string, CallbackType> actionMapping_;
	std::vector<AsyncWorkerPtr> asyncWorkers_;
	std::shared_ptr<SyncWorker> syncWorker_;
};