#pragma once

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