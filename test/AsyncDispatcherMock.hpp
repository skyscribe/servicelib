#pragma once
#include "AsyncDispatcher.h"
#include "gmock/gmock.h"

class AsyncDispatcherMock : public AsyncDispatcher{
public:
    AsyncDispatcherMock(AsyncWorkerQueue queue = AsyncWorkerQueue()) : AsyncDispatcher(queue){};
    virtual ~AsyncDispatcherMock(){};

    MOCK_METHOD1(start, void(size_t));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD5(scheduleJob, bool(const std::string&, Callable, Callable, bool, const std::string&));
    MOCK_METHOD1(cancelJobsFor, void(const std::string&));
};