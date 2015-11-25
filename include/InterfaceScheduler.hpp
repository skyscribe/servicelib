#pragma once 
#include "ArgDefs.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>

class AsyncDispatcher;
class InterfaceScheduler{
public:
	InterfaceScheduler(AsyncWorkerQueue queue = AsyncWorkerQueue(), std::shared_ptr<SyncWorker> sync = nullptr);
	virtual ~InterfaceScheduler(){}

	static const size_t defaultPoolSize = 4;
	virtual void start(size_t asyncPoolSize = defaultPoolSize);
	virtual void stop();

	virtual void registerInterface(const std::string& idStr, CallbackType action, std::string typeId);
	virtual void unRegiterInterface(const std::string& idStr);
	virtual bool isServiceRegistered(const std::string& idStr) const;
	//Setup a watcher for interface registration - cb shall never block!
	virtual void subscribeForRegistration(const std::string& idStr, std::function<void()>&& cb);

	//Schedule a previously registered interface cally by CallProperty (see its definition)
	template <class ... Args>
	bool interfaceCall(const std::string& idStr, CallProperty&& prop, const Args& ...args){
		return checkAndInvokeCall<Args ...>(idStr, prop.async, prop.waitForDone, 
			std::forward<Callable>(prop.onCallDone), prop.strand, args...);
	}

private:
	InterfaceScheduler& operator=(const InterfaceScheduler&) = delete;
	void createWorkersIfNotInitialized(size_t poolSize);

	//Actual call under the hood
	template <class ... Args>
	inline bool checkAndInvokeCall(const std::string& idStr, bool async, bool waitForDone, 
			Callable&& onCallDone, const std::string& strand, const Args& ... args){
		typedef ParamArgs<Args ...> ActualType;
		CallbackType action;
		if(!isCallRegisteredAndTypesMatch(idStr, ActualType::getType(), action))
			return false;

		return invokeCall([args..., action]() -> bool{
			ActualType param(args...);
			return action(param);
		}, async, waitForDone, strand, std::forward<Callable>(onCallDone));			
	}

	bool fetchStoredCallbackByServiceId(const std::string& idStr, CallbackType& call, std::string& typeStr);
	virtual bool isCallRegisteredAndTypesMatch(const std::string& idStr, const std::string&& callType,
		CallbackType& action);

	//Invoke the actual call wrapped in cb/onDone
	virtual bool invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& strand, Callable&& onDone);

	//Notify all subscribers - not protected
	void notifySubscribersOnRegistration(const std::string& id);

	typedef std::pair<CallbackType, std::string> CallbackState;
	std::unordered_map<std::string, CallbackState> actionMapping_;
	std::unordered_map<std::string, std::vector<std::function<void()>>> notifyMapping_;
	mutable std::recursive_mutex mappingLock_;

	std::shared_ptr<SyncWorker> syncWorker_;
	std::atomic<bool>			started_;

	std::shared_ptr<AsyncDispatcher> asyncDispather_;
};