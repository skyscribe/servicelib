#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Worker.h"

using namespace std::chrono_literals;

TEST(workerTest, cancelOngoingJob_cancelOperationBlockedUntilJobReturned){
    std::recursive_mutex lock;
    std::vector<int> steps;
    auto markStep = [&](int step) -> void{
        std::lock_guard<std::recursive_mutex> guard(lock);
        steps.push_back(step);
    };

    std::vector<int> exps = {1,2,3,4};
    std::atomic<bool> running{false};

    AsyncWorker worker;
    worker.blockUntilReady();

    worker.doJob("test", [&]() -> bool{
                markStep(2);
                running = true;
                //TODO: how to ensure this happens and blocks cancel 
                markStep(3);
                return true;
            }, []() -> bool{
                return true;
            });

    markStep(1);
    while(!running){
        std::this_thread::yield();
    }
    worker.cancelJobsFor("test");
    markStep(4);

    worker.stop();
    EXPECT_THAT(steps, ::testing::Eq(exps));   
}
