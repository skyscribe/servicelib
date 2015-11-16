#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>

#include "InterfaceScheduler.hpp"
#include "WorkerMock.hpp"

using namespace std;

class SchedulerMockTest : public ::testing::Test{
protected:
	shared_ptr<AsyncWorkerMock> asyncs_;
	shared_ptr<SyncWorkerMock> sync_;
	InterfaceScheduler sched_;

	SchedulerMockTest() : Test(), asyncs_(make_shared<AsyncWorkerMock>()), sync_(make_shared<SyncWorkerMock>()),
			sched_({asyncs_}, sync_){}
};

TEST_F(SchedulerMockTest, startTwice_duplicateStartIgnored){
	EXPECT_CALL(*asyncs_, blockUntilReady()).Times(1);
	sched_.start(1);
	sched_.start(1);
}

TEST_F(SchedulerMockTest, invokeNotRegisteredCall_NothingCalled){
	//TODO: catch exception and check the throw behavior - shall have assertion error!
	using ::testing::_;
	EXPECT_CALL(*sync_, doJob(::testing::_, ::testing::_)).Times(0);
	EXPECT_FALSE(sched_.interfaceCall("unknown"));
}