#include "Helper.hpp"
#include <iostream>
#include <thread>
#include <tuple>
#include <cassert>
#include <chrono>
#include <algorithm>
using namespace std;

InterfaceScheduler* createTestScheduler(){
    assert(!"This shall never be called in real binary!");
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
double getFactorial(int n){
    double ret = 1;
    for (auto i = 1; i <= n; ++i)
        ret *= i;
    return ret;
}
const std::string serviceName = "calculate";
class SubCalculator{
public:
    SubCalculator(){
        registerInterfaceFor<int, int>(serviceName, [this](const auto& p){
            return this->calculate(p);
            });
        //cout << "register service:" << serviceName << endl;
    }

    bool calculate(const ParamArgs<int, int>& p){
        double result = 0.0;
        for (auto i = get<0>(p); i < get<1>(p); ++i){
            result += 1.0/getFactorial(i);
            //make this slow
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::lock_guard<std::mutex> guard(mutex_);
        results_.push_back(result);
        cout << "calculated as:" << result << endl;
        return true;
    }

    std::vector<double> getResults()const{return results_;}
private:
    std::mutex mutex_;
    std::vector<double> results_;
};

///////////////////////////////////////////////////////////////////////////////
std::thread startCalculator(atomic<bool>& stop, std::unique_ptr<SubCalculator>& serv){
    //start service asynchronously
    std::thread thr([&]{
        //cout << "Creating and binding service " << serviceName << endl;
        serv.reset(new SubCalculator());
        while (!stop)
            this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    return thr;   
}

void subscribeAndStart(atomic<int>& finished){
    //calculated results will be stored in a list
    auto onDone = [&]()->bool{
        finished++;
        return true;
    };
    //subscribe and call
    getGlobalScheduler().subscribeForRegistration(serviceName, [&]{
        //cout << "registered now..." << endl;
        for (int i = 0; i < 4; ++i)
            asyncCall(serviceName, std::forward<Callable>(onDone), i*10, (i+1)*10);
    });  
}

void waitForAllFinished(atomic<int>& finished, unique_ptr<SubCalculator>& serv){
    //e = 1 + 1/1! + 1/2! + 1/3! + 1/4! + ... + 1/n!
    while (finished != 4){
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    auto results = std::move(serv->getResults());
    cout << "calculated e = " << std::accumulate(results.begin(), results.end(), 0.0) << endl;   
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    cout.precision(32);
    getGlobalScheduler().start(4);

    std::atomic<bool> stop(0);
    std::atomic<int> finished(0);
    std::unique_ptr<SubCalculator> serv;
    thread thr = startCalculator(stop, serv);
    subscribeAndStart(finished);
    waitForAllFinished(finished, serv);

    cout << "exiting..." << endl;
    stop = 1;
    thr.join();
    getGlobalScheduler().stop();
}