#include "AsyncDispatcherMock.hpp"
#include "InterfaceScheduler.hpp"
#include "Helper.hpp"

using namespace std;

class StartTest : public ::testing::Test{
protected:
    shared_ptr<InterfaceScheduler> sched_;
    shared_ptr<AsyncDispatcherMock> disp_;
    StartTest() : ::testing::Test(){
        disp_ = make_shared<AsyncDispatcherMock>();
        sched_ = make_shared<InterfaceScheduler>(disp_);
    }
};

TEST_F(StartTest, startTwice_duplicateStartIgnored){
    EXPECT_CALL(*disp_, start(::testing::_)).Times(1);
    sched_->start(1);
    sched_->start(1);
}

const string serviceName = "service";
class CancelTest : public StartTest{};
TEST_F(CancelTest, unregisterUnknowService_NothingCancelled){
    EXPECT_CALL(*disp_, cancelJobsFor(serviceName)).Times(0);
    sched_->unRegiterInterface(serviceName);
}

TEST_F(CancelTest, unregisterRegistered_jobsCancelled){
    EXPECT_CALL(*disp_, cancelJobsFor(serviceName)).Times(1);
    registerInterfaceFor<int>(*sched_, serviceName, [](const ParamArgs<int>&){return true;});
    sched_->unRegiterInterface(serviceName);
}