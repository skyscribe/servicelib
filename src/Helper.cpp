#include "Helper.hpp"
#include "InterfaceScheduler.hpp"
#include <atomic>

using namespace std;
namespace{
    InterfaceScheduler* theScheduler;
    std::once_flag creationFlag;
    static std::atomic<bool> created(0);
    static bool defaultForTesting = false;
}

void setGlobalSchedulerPurpose(bool forTesting){
    defaultForTesting = forTesting;
}

extern InterfaceScheduler* createTestScheduler();

InterfaceScheduler& getGlobalScheduler(){
    if (!created)
        std::call_once(creationFlag, []{
            if (defaultForTesting)
                theScheduler = createTestScheduler();//new MockedInterfaceScheduler();
            else
                theScheduler = new InterfaceScheduler();
            created = true;
        });
    return *theScheduler;
}

void releaseDefaultScheduler(){
    if (created){
        delete theScheduler;
        theScheduler = nullptr;
        created = false;
    }
}