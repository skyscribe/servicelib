#pragma once
#include "AsyncDispatcher.h"
#include "gmock/gmock.h"

class AsyncDispatcherMock : public AsyncDispatcher{
public:
    AsyncDispatcherMock() : AsyncDispatcher({}){};
    virtual ~AsyncDispatcherMock(){};

    MOCK_METHOD1(start, void(size_t));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD4(scheduleJob, bool(bool, const std::string&, Callable, Callable));
};