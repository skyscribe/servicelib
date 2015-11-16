#include "gmock/gmock.h"
#include "Worker.h"
class SyncWorkerMock : public SyncWorker{
public:
	MOCK_METHOD2(doJob, bool(Callable, Callable));	
};

class AsyncWorkerMock : public AsyncWorker{
public:
	MOCK_METHOD0(blockUntilReady, void());
	MOCK_METHOD0(stop, void());
	MOCK_METHOD2(doJob, bool(Callable, Callable));
	MOCK_METHOD2(doSyncJob, bool(Callable, Callable));
	MOCK_METHOD0(getLoad, size_t());
	MOCK_METHOD0(getId, std::thread::id());
};