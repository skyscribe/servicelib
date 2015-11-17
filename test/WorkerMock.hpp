#include "gmock/gmock.h"
#include "Worker.h"
#include <memory>

class SyncWorkerMock : public SyncWorker{
public:
	MOCK_METHOD2(doJob, bool(Callable, Callable));	
};

class AsyncWorkerMock : public AsyncWorker{
public:
	AsyncWorkerMock(bool callReal = false);

	MOCK_METHOD0(blockUntilReady, void());
	MOCK_METHOD0(stop, void());
	MOCK_METHOD2(doJob, bool(Callable, Callable));
	MOCK_METHOD2(doSyncJob, bool(Callable, Callable));
	MOCK_METHOD0(getLoad, size_t());
	MOCK_METHOD0(getId, std::thread::id());

private:
	void increaseLoad();
	size_t fetchLoad();
	size_t load_;
	std::shared_ptr<AsyncWorker> real_;
};