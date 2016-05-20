#pragma once
#include "InterfaceScheduler.hpp"
#include "Helper.hpp"
#include "gmock/gmock.h"

class MockedInterfaceScheduler : public InterfaceScheduler{
public:
    virtual ~MockedInterfaceScheduler(){};
    MockedInterfaceScheduler(){
        using ::testing::_;
        ON_CALL(*this, isCallRegisteredAndTypesMatch_RVR(_, _, _)).WillByDefault(::testing::Return(true));
    }

    MOCK_METHOD1(start, void(size_t));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD3(registerInterface, void(const std::string&, CallbackType, std::string));
    MOCK_METHOD1(unRegiterInterface, void(const std::string&));
    MOCK_METHOD1(isServiceRegistered, bool(const std::string&));

    //To workaround rvalue reference issue in gmock(not supported), add a proxy
    // see also in: http://stackoverflow.com/questions/12088537/workaround-for-gmock-to-support-rvalue-reference
    void subscribeForRegistration(const std::string& id, std::function<void()>&& f) override{
        subscribeForRegistration_RVR(id, f);
    }
    MOCK_METHOD2(subscribeForRegistration_RVR, void(const std::string&, std::function<void()>));

    //workaround the template impl. in parent class: explicitly overwrite this
    bool isCallRegisteredAndTypesMatch(const std::string& idStr, const std::string&& callType,
        CallbackType& action)override{return isCallRegisteredAndTypesMatch_RVR(idStr, callType, action);}
    MOCK_METHOD3(isCallRegisteredAndTypesMatch_RVR, bool(const std::string&, const std::string&, CallbackType&));

    virtual bool invokeCall(Callable&& cb, bool async, bool waitForDone, const std::string& id,
            const std::string& strand, Callable&& onDone) override{
        return invokeCall_RVR(cb, async, waitForDone, id, strand, onDone);
    }
    MOCK_METHOD6(invokeCall_RVR, bool(Callable, bool, bool, const std::string&, const std::string&, Callable));
};

class SchedulerFixture : public ::testing::Test{
protected:
    MockedInterfaceScheduler* sched_;
    virtual void SetUp() override{
        releaseGlobalScheduler();
        setGlobalSchedulerAsMocked(true); //always reset for testing purpose
        MockedInterfaceScheduler& sched = dynamic_cast<MockedInterfaceScheduler&>(getGlobalScheduler());
        sched_ = &sched;
    }

    virtual void TearDown() override{
        releaseGlobalScheduler();
    }
};
