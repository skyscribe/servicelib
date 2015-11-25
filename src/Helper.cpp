#include "Helper.hpp"
#include "InterfaceScheduler.hpp"
#include <atomic>

using namespace std;
namespace{
    InterfaceScheduler* theScheduler;
    static std::once_flag* accessFlag = new once_flag();
    static std::atomic<bool> created(0);
    static std::mutex sharedMutex;
    static bool defaultForTesting = false;
}

void setGlobalSchedulerPurpose(bool forTesting){
    defaultForTesting = forTesting;
}

extern InterfaceScheduler* createTestScheduler();

InterfaceScheduler& getGlobalScheduler(){
    auto createScheduler = []() -> InterfaceScheduler*{
        if (defaultForTesting)
            return createTestScheduler();
        else
            return new InterfaceScheduler();
    };
    if (!created){
        std::lock_guard<std::mutex> guard(sharedMutex);
        theScheduler = createScheduler();
        created = true;
    }
    return *theScheduler;
}

void releaseDefaultScheduler(){
    if (created){
        std::lock_guard<std::mutex> guard(sharedMutex);
        delete theScheduler;
        theScheduler = nullptr;
        created = false;
    }
}