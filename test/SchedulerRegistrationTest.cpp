#include "Helper.hpp"
#include "SchedulerTestFixture.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <thread>
using namespace std;

//Need stress test for concurrency check!
class SchedulerRegistrationTest : public SchedulerTest{
protected:
    std::vector<int> steps_;
    const std::vector<int> exp_ = {1,2,3,4};
    const std::string name_ = "service"; //doesn't matter
    std::mutex stepsMutex_;
    std::function<bool(const ParamArgs<int>&)> action_;

    void appendStep(int i){
        std::lock_guard<mutex> guard(stepsMutex_);
        steps_.push_back(i);
    }

    void invokeCallAndCheckSteps(){
        asyncCall(sched_, name_, 4);
        sched_.stop();
        EXPECT_THAT(steps_, ::testing::Eq(exp_));   
    }

    virtual void SetUp() override{
        action_ = std::bind(&SchedulerRegistrationTest::markStepAction,
            this, std::placeholders::_1);
        SchedulerTest::SetUp();
    }

public:
    bool markStepAction(const ParamArgs<int>& p){
        appendStep(get<0>(p));
        return true;
    }
};

TEST_F(SchedulerRegistrationTest, concurrentRegister_registeredSuccessfully){
    atomic<int> registered(0);

    auto service = [&](const ParamArgs<int>& par){ registered += get<0>(par); return true;};
    std::vector<shared_ptr<thread>> threads;
    const size_t jobs = 8;

    for (auto i = 0; i < jobs; ++i)
        threads.push_back(make_shared<thread>([&, i]() -> void{
            stringstream strm;
            strm << "serv" << i; //to_string not supported on cygwin?
            registerInterfaceFor<int>(sched_, strm.str(), service);
            CallProperty prop = {true, true, Callable(), ""};
            EXPECT_TRUE(sched_.interfaceCall(strm.str(), forward<CallProperty>(prop), i));
        }));

    for (auto inst : threads)
        inst->join();
    sched_.stop();

    auto getAccumulation = [](int start, int end)->int{
        int result = 0;
        for (auto i = start; i < end; ++i)
            result += i;
        return result;
    };
    EXPECT_EQ(registered, getAccumulation(0, jobs));
}

TEST_F(SchedulerRegistrationTest, subscribeForInterfaceRegisterd_notifiedOnRegistration){
    appendStep(1);
    sched_.subscribeForRegistration(name_, [&]{appendStep(3);});
    appendStep(2);
    registerInterfaceFor<int>(sched_, name_, action_);
    invokeCallAndCheckSteps();
}

TEST_F(SchedulerRegistrationTest, subscribeForRegisteredInterface_notifiedImmediately){
    appendStep(1);
    registerInterfaceFor<int>(sched_, name_, action_);
    sched_.subscribeForRegistration(name_, [&]{appendStep(2);});
    appendStep(3);
    invokeCallAndCheckSteps();
}

TEST_F(SchedulerRegistrationTest, registerTwiceForSameInterface_secondTryShallThrow){
    registerInterfaceFor<int>(sched_, name_, action_);
    EXPECT_THROW(registerInterfaceFor<int>(sched_, name_, action_), std::logic_error);
}
