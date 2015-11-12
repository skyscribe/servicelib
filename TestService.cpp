#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Worker.h" //required for unique_ptr
#include <iostream>

int main()
{
	InterfaceScheduler sched;
	DemoService serv(sched);
	sched.start();

	sched.interfaceCall("doSomethingA", false, true, Callable(), 2, "synchronous call");
	sched.interfaceCall("doSomethingB", true, false, []()->bool{
		std::cout << "Calling done" << std::endl;
	}, 3, "asynchrous call");

	return 0;
}