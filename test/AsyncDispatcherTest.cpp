#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>

#include "AsyncDispatcher.h"
#include "WorkerMock.hpp"

using namespace std;

class AsyncDispatcherTest : public ::testing::Test{
protected:
	shared_ptr<AsyncWorkerMock> asyncs_;
	AsyncDispatcher dispatcher_;

	AsyncDispatcherTest() : Test(), asyncs_(make_shared<AsyncWorkerMock>()),
			dispatcher_({asyncs_}){}
};

