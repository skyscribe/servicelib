#pragma once
#include <functional>
#include <tuple>
#include <memory>
#include <vector>
#include <typeinfo>

struct ParaArgsBase{};

template <class ... Args>
struct ParamArgs : public ParaArgsBase{
    ParamArgs(const Args&... args) : parameters(args...){}
    std::tuple<Args...> parameters;
    static const char* getType(){return typeid(std::tuple<Args...>).name();}
};

template <std::size_t I, class ... Types>
typename std::tuple_element<I, std::tuple<Types...> >::type const& get(const ParamArgs<Types ...>& args){
    return std::get<I>(args.parameters);
}

typedef std::function<bool(const ParaArgsBase&)> CallbackType;
typedef std::function<bool()> Callable;

struct CallProperty{
    bool async; //will the job be scheduled asynchronously (under same context)
    bool waitForDone; //if interfaceCall will be blocked (for done) or not, will be ignored for async call
    Callable onCallDone;
    std::string strand; //calls with same strand will be scheduled by same thread, async only
};

inline CallProperty createAsyncNonBlockProp(Callable&& onDone, const std::string& strand = ""){
    CallProperty prop = {true, false, onDone, strand};
    return prop;
}

class AsyncWorker;
typedef std::shared_ptr<AsyncWorker> AsyncWorkerPtr;
class SyncWorker;
typedef std::vector<AsyncWorkerPtr> AsyncWorkerQueue;