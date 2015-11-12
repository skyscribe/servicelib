#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Worker.h" //required for unique_ptr
#include <iostream>

int main()
{
	InterfaceScheduler sched;
	DemoService serv(sched);
	sched.start();

	std::string hint("synchronous call");
	sched.interfaceCall("doSomethingA", false, true, Callable(), 2, hint);
	sched.interfaceCall("doSomethingB", true, false, []()->bool{
		std::cout << "Calling B asynchronously done" << std::endl;
	}, 3, std::string("asynchrous call"));

	return 0;
}