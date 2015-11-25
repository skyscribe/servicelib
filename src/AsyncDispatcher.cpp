#include "AsyncDispatcher.h"
#include "Worker.h"
#include <cassert>
#include <iostream>
#include <algorithm>

using namespace std;

bool AsyncDispatcher::scheduleJob(const std::string& name, Callable action, Callable onDone, bool waitForDone,
        const std::string& strand){
    if (!waitForDone)
        return getWorkerForSchedule(strand)->doJob(name, std::forward<Callable>(action),
                std::forward<Callable>(onDone));
    else
        return getWorkerForSchedule(strand)->doSyncJob(name, std::forward<Callable>(action),
                std::forward<Callable>(onDone));  
}

const AsyncWorkerPtr& AsyncDispatcher::getWorkerForSchedule(const std::string& strand){
    auto it = min_element(asyncWorkers_.begin(), asyncWorkers_.end(), 
        [](const AsyncWorkerPtr& a, const AsyncWorkerPtr& b) -> bool{
            return a->getLoad() < b->getLoad();
    });
    //dumpWorkersLoad(cout);
    if (strand.empty())
        return *it;
    else
        return getStrandWorkerForSchedule(strand, *it);
}

const AsyncWorkerPtr& AsyncDispatcher::getStrandWorkerForSchedule(const std::string& strand,
        const AsyncWorkerPtr& idle){
    std::lock_guard<mutex> guard(strandsLock_);
    if (strands_.find(strand) == strands_.end())
        strands_[strand] = idle;
    return strands_[strand];
}

void AsyncDispatcher::start(size_t poolSize){
    createWorkersIfNotInitialized(poolSize);
    for (auto worker : asyncWorkers_)
        worker->blockUntilReady();
}

void AsyncDispatcher::createWorkersIfNotInitialized(size_t poolSize){
    if (asyncWorkers_.empty())
        for (auto i = 0; i < poolSize; ++i)
            asyncWorkers_.push_back(make_shared<AsyncWorker>());
    else
        assert(asyncWorkers_.size() == poolSize);   
}

void AsyncDispatcher::stop(){
    for(auto worker : asyncWorkers_)
        worker->stop();
}

void AsyncDispatcher::dumpWorkersLoad(std::ostream& collector)const{
    collector << "AsyncWorkers load info: total=" << asyncWorkers_.size() << endl;
    for (auto worker : asyncWorkers_)
        collector << "\tWorker<" << worker->getId() << ",load=" << worker->getLoad() << endl;  
}

void AsyncDispatcher::getStatistics(size_t& asyncWorksCnt, size_t& totalLoad){
    asyncWorksCnt = asyncWorkers_.size();
    totalLoad = 0;
    for (auto worker : asyncWorkers_)
        totalLoad += worker->getLoad();
}

void AsyncDispatcher::cancelJobsFor(const std::string& idStr){
    return;
}
