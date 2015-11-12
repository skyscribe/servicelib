#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Worker.h" //required for unique_ptr
#include <iostream>

using namespace std;

int main()
{
	InterfaceScheduler sched;
	DemoService serv(sched);
	sched.start();

	std::string hint("synchronous call");
	cout << "### Calling doSomethingA synchronously by sync interface" << endl;
	sched.interfaceCall("doSomethingA", false, true, Callable(), 2, hint);

	cout << "### Calling doSomethingB synchronously by async interface" << endl;
	sched.interfaceCall("doSomethingB", true, true, []()->bool{
		std::cout << "Calling B synchronously through async interface done" << std::endl;
	}, true, 133);

	cout << "### Calling doSomethingB asynchronously by async interface" << endl;
	sched.interfaceCall("doSomethingB", true, false, []()->bool{
		std::cout << "Calling B asynchronously done" << std::endl;
	}, false, 131);
	cout << "### calling done!" << endl;

	return 0;
}