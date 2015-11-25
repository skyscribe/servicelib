#pragma once
#include "gtest/gtest.h"
#include "InterfaceScheduler.hpp"
#include "DemoService.hpp"
#include "Helper.hpp"

class IntegrationTest : public ::testing::Test{
protected:
	InterfaceScheduler sched_;
	DemoService service_;

	IntegrationTest() : Test(), service_(sched_) {}
	virtual void SetUp() override{
		sched_.start(1);
	}
	virtual void TearDown() override{
		sched_.stop(); 
	}
};