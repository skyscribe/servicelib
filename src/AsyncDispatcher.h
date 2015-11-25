#pragma once
#include "ArgDefs.hpp"
#include <mutex>
#include <unordered_map>

//AsyncDispatcher holds all asynchronous workers and dispatch jobs to a certain worker
// strand policy is supported so user can specify which worker they want to attach a job to
class AsyncDispatcher{
public:
    AsyncDispatcher(AsyncWorkerQueue queue) : asyncWorkers_(queue){};
    virtual ~AsyncDispatcher(){}

    virtual void start(size_t poolSize);
    virtual void stop();
    virtual bool scheduleJob(bool waitForDone, const std::string& strand, Callable action, Callable onDone);

    void dumpWorkersLoad(std::ostream& collector)const;
    void getStatistics(size_t& asyncWorksCnt, size_t& totalLoad);
private:
    AsyncDispatcher& operator=(const AsyncDispatcher&) = delete;

    void createWorkersIfNotInitialized(size_t poolSize);
    const AsyncWorkerPtr& getWorkerForSchedule(const std::string& strand);
    const AsyncWorkerPtr& getStrandWorkerForSchedule(const std::string& strand, const AsyncWorkerPtr& idle);

    AsyncWorkerQueue asyncWorkers_;
    std::unordered_map<std::string, AsyncWorkerPtr> strands_;
    mutable std::mutex strandsLock_;
};