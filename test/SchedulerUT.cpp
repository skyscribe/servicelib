#include "AsyncDispatcherMock.hpp"
#include "InterfaceScheduler.hpp"

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