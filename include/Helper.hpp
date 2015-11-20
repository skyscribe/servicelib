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
	typedef ParamArgs<Args ...> ActualType;
	static_assert(std::is_convertible<ActionType, std::function<void(const ActualType&)>>::value,
		"Incompatible type!"); 
	sched.registerInterface(idStr, [=](const ParaArgsBase& p) -> bool{
		return action(static_cast<const ActualType&>(p));
	});
}