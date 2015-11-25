#include "InterfaceSchedulerMock.hpp"
#include "Helper.hpp"

typedef SchedulerFixture SingletonTesting;
TEST_F(SingletonTesting, getGlobalScheduler_schedulerCreatedOnFirstCall){
    using ::testing::_;
    EXPECT_CALL(*sched_, registerInterface("example", ::testing::_, ::testing::_)).Times(1);
    EXPECT_CALL(*sched_, invokeCall_RVR(_, true /*async*/, false/*nonblock*/, "example", _, _)).Times(1);
    EXPECT_CALL(*sched_, isCallRegisteredAndTypesMatch_RVR(_, _, _)).Times(1);

    registerInterfaceFor<int>("example", [](const ParamArgs<int>&)->bool{ return true;});
    asyncCall("example", 1);
}