#include "InterfaceScheduler.hpp"
#include <iostream>
#include <string>
#include <sstream>

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
		std::stringstream strm;
		strm << "Method A executed, with parameters:" 
			<< get<0>(args) << "," << get<1>(args)
			<< std::endl;
		result_ = strm.str();
	}

	bool doSomethingB(const ParamArgs<bool, int>& args){
		std::stringstream strm;
		strm << "Method B executed with parameters:"
			<< std::get<0>(args.parameters) << "," << std::get<1>(args.parameters)
			<< std::endl;
		result_ = strm.str();
	}

	std::string getResult(){return result_;}

private:
	InterfaceScheduler& scheduler_;
	std::string result_;
};