#pragma once 

#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>
#include <memory>

struct ParaArgsBase{};

template <class ... Args>
struct ParamArgs : public ParaArgsBase{
	ParamArgs(Args&&... args) : parameters(std::forward<Args>(args)...){}
	std::tuple<Args...> parameters;
};

typedef std::function<bool(const ParaArgsBase&)> CallbackType;
typedef std::function<bool()> Callable;
class AsyncWorker;
class SyncWorker;

class InterfaceScheduler{
public:
	InterfaceScheduler();

	void start();
	void stop();

	void registerInterface(const std::string& idStr, CallbackType action);
	void unRegiterInterface(const std::string& idStr);

	template <class ... Args>
	bool interfaceCall(const std::string& idStr, bool async = false, bool waitForDone = true,
			Callable&& onCallDone = Callable(), Args&& ... args){
		auto actIt = actionMapping_.find(idStr);
		if (actIt == actionMapping_.end()){
			std::cout << "Non-existent interface to call:" << idStr << std::endl;
			return false;
		}else{
			ParamArgs<Args ...> param(std::forward<Args>(args)...);
			auto callable = [=, &actIt]() -> bool{
				return actIt->second(param);
			};
			return invokeCall(callable, async, waitForDone, std::forward<Callable>(onCallDone));
		}
	}

private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;

	bool invokeCall(Callable&& cb, bool async, bool waitForDone, Callable&& onDone);
	std::unordered_map<std::string, CallbackType> actionMapping_;
	std::unique_ptr<AsyncWorker> asyncWorker_;
	std::unique_ptr<SyncWorker> syncWorker_;
};
