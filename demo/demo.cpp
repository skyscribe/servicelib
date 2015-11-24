#include "Helper.hpp"
#include <iostream>
#include <thread>
#include <tuple>
#include <cassert>
#include <chrono>
using namespace std;

InterfaceScheduler* createTestScheduler(){
    assert(!"This shall never be called in real binary!");
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
const std::string serviceName = "fetchState";
class ServiceImpl{
public:
    ServiceImpl(){
        for (auto i = 1; i < 10; ++i)
            data_[i] = 1 << i;
        registerInterfaceFor<int>(serviceName, std::bind(&ServiceImpl::fetchState,
            this, std::placeholders::_1));
        cout << "register service:" << serviceName << endl;
    }

    bool fetchState(const ParamArgs<int>& p){
        cout << "calling with " << get<0>(p) << endl;
        return true;
    }

private:
    std::unordered_map<int, int> data_;
};

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{   
    getGlobalScheduler().start(1);
    std::atomic<bool> stop(0);
    std::unique_ptr<ServiceImpl> serv;

    //start service asynchronously
    std::thread thr([&]{
        cout << "Creating and binding service " << serviceName << endl;
        serv.reset(new ServiceImpl());
        cout << "serving..." << endl;
        while (!stop)
            this_thread::sleep_for(std::chrono::milliseconds(5));
        cout << "Exiting..." << endl;
    });

    //subscribe and call
    getGlobalScheduler().subscribeForRegistration(serviceName, []{
        cout << "registered now..." << endl;
        asyncCall(serviceName, 2);
    });

    //exit all
    stop = 1;
    thr.join();
    getGlobalScheduler().stop();
}