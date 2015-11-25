#pragma once
#include "AsyncDispatcher.h"
#include "gmock/gmock.h"

class AsyncDispatcherMock : public AsyncDispatcher{
public:
    AsyncDispatcherMock(AsyncWorkerQueue queue = AsyncWorkerQueue()) : AsyncDispatcher(queue){};
    virtual ~AsyncDispatcherMock(){};

    MOCK_METHOD1(start, void(size_t));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD4(scheduleJob, bool(bool, const std::string&, Callable, Callable));
    MOCK_METHOD1(cancelJobsFor, void(const std::string&));
};