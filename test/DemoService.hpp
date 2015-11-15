#include "InterfaceScheduler.hpp"
#include <iostream>
#include <string>
#include <sstream>

class DemoService{
public:
	DemoService(InterfaceScheduler& sched) : scheduler_(sched){
		scheduler_.registerInterfaceFor<int, std::string>("doSomethingA", 
			std::bind(&DemoService::doSomethingA, this, std::placeholders::_1));
		scheduler_.registerInterfaceFor<bool, int>("doSomethingB",
			std::bind(&DemoService::doSomethingB, this, std::placeholders::_1));
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