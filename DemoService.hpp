#include "InterfaceScheduler.hpp"
#include <iostream>
#include <string>

class DemoService{
public:
	DemoService(InterfaceScheduler& sched) : scheduler_(sched){
		scheduler_.registerInterface("doSomethingA", [this](const ParaArgsBase& arg) -> bool{
			return this->doSomethingA(static_cast<const ParamArgs<int, std::string>&>(arg));
		});
		scheduler_.registerInterface("doSomethingB", [this](const ParaArgsBase& arg) -> bool{
			return doSomethingB(static_cast<const ParamArgs<bool, int>&>(arg));
		});
	}

	bool doSomethingA(const ParamArgs<int, std::string>& args){
		int theInt;
		std::string theStr;
		std::tie(theInt, theStr) = args.parameters;
		std::cout << "Method A executed, with parameters:" 
			<< theInt << "," << theStr
			<< std::endl;
	}

	bool doSomethingB(const ParamArgs<bool, int>& args){
		std::cout << "Method B executed with parameters:"
			<< std::get<0>(args.parameters) << "," << std::get<1>(args.parameters)
			<< std::endl;
	}

private:
	InterfaceScheduler& scheduler_;
};