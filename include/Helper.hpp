#pragma once
#include "InterfaceScheduler.hpp"
#include <functional>
#include <string>
#include <chrono>
#include <iostream>
#include <type_traits>

//profiler function
inline size_t profileFor(std::function<void()> call, const std::string& hint = "", bool print = false){
	using namespace std::chrono;
	auto start = steady_clock::now();
	call();
	auto diff = duration_cast<milliseconds>(steady_clock::now() - start);
	if (print)	
		std::cout << "Time spent on <" << hint << "> is " << diff.count() << " milliseconds" << std::endl;
	return diff.count();
};

//The default scheduler that might be shared by multiple clients if they want to, singleton object is only created
// on first call (lazy creation)
InterfaceScheduler& getGlobalScheduler();
//The default scheduler can be released explictly also
void releaseGlobalScheduler();
void setGlobalSchedulerAsMocked(bool mocked = false);

template <class ... Args, class ActionType>
inline void registerInterfaceFor(const std::string& idStr, ActionType action){
	registerInterfaceFor<Args ...>(getGlobalScheduler(), idStr, std::move(action));
}

template <class ... Args, class ActionType>
inline void registerInterfaceFor(InterfaceScheduler& sched, const std::string& idStr, ActionType action){
	//Check type-safety as possible, lambdas/binds shall have targets, while functions may not
	typedef ParamArgs<Args ...> ActualType;
	typedef std::function<bool(const ActualType&)> FuncType;
	static_assert(std::is_convertible<ActionType, FuncType>::value, "Incompatible type!");
	
	//lambdas/mem_func may not define operator bool() to check - explicit convert as a workwaround
	FuncType func(action); 
	if (!func)
		throw std::invalid_argument("Null action specified for interface:" + idStr);

	sched.registerInterface(idStr, [=](const ParaArgsBase& p) -> bool{
		return func(static_cast<const ActualType&>(p));
	}, ActualType::getType());
}

//example call: asyncCall(sched, "domain.serviceId", arg1, arg2, ...);
template <class ...Args>
bool asyncCall(InterfaceScheduler& sched, const std::string& serviceId, const Args& ... args){
	return sched.interfaceCall(serviceId, createAsyncNonBlockProp(Callable()), args...);
}

template <class ...Args>
bool asyncCall(InterfaceScheduler& sched, const std::string& serviceId, Callable&& onDone, const Args& ... args){
	return sched.interfaceCall(serviceId, createAsyncNonBlockProp(std::forward<Callable>(onDone)), args...);
}

template <class ...Args>
bool asyncCall(const std::string& serviceId, const Args& ... args){
	return asyncCall(getGlobalScheduler(), serviceId, args...);
}

template <class ...Args>
bool asyncCall(const std::string& serviceId, Callable&& onDone, const Args& ... args){
	return asyncCall(getGlobalScheduler(), serviceId, std::forward<Callable>(onDone), args...);
}