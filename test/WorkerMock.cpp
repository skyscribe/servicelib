#include "WorkerMock.hpp"
using ::testing::Invoke;
using ::testing::_;
using ::testing::DoAll;
using ::testing::WithoutArgs;

AsyncWorkerMock::AsyncWorkerMock(bool callReal) : load_(0){
	if (callReal){
		real_ = std::make_shared<AsyncWorker>();
		ON_CALL(*this, blockUntilReady()).WillByDefault(Invoke(real_.get(), &AsyncWorker::blockUntilReady));
		ON_CALL(*this, stop()).WillByDefault(Invoke(real_.get(), &AsyncWorker::stop));
		ON_CALL(*this, doJob(_, _)).WillByDefault(
			DoAll(WithoutArgs(Invoke(this, increaseLoad)), 
				Invoke(real_.get(), &AsyncWorker::doJob)));
		ON_CALL(*this, getLoad()).WillByDefault(Invoke(this, &AsyncWorkerMock::fetchLoad));
		ON_CALL(*this, getId()).WillByDefault(Invoke(real_.get(), &AsyncWorker::getId));		
	}
}

size_t AsyncWorkerMock::fetchLoad(){
	return load_;
}

void AsyncWorkerMock::increaseLoad(){
	load_++;
}