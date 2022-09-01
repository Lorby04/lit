#ifndef __LI_SERVICE__
#define __LI_SERVICE__

#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <memory>
#include "channel.h"
#include "li_target.h"

using namespace std;
enum OP{
    ADD,
    QUERY,
    DEL,
    STOP,
    NA
};

class Message{
public:    
    std::unique_ptr<Target> mTarget;
    OP mOp;
public:
    Message(std::unique_ptr<Target> aTarget, OP aOp):
        mTarget(std::move(aTarget)), mOp(aOp) {}
};

class Service{
    Channel<Message> *mChannel;
    int mThreads;
    uint64_t mTotalEntries;

    atomic_uint64_t mQueryAttemptCount;
    atomic_uint64_t mFoundCount;

    static std::once_flag mfInst;
    static Service* mInstance;

private:    
    Service(int n);
    void start();
    static void create(int n){
        Service::mInstance = new Service(n);
    }
    void worker(int aId);
    string statistics() ;
    bool query(Target &aTarget);
public: 
    static Service *init(int n){
        std::call_once(Service::mfInst,Service::create,n);
        return Service::mInstance;
    }
    static Service *getInstance(){
        return Service::mInstance;
    }
    static void thread_entry(Service *aInst, int aId){
        aInst->worker(aId);
    }
public:
    void generateTargets(uint64_t n);
    void perfTest(int n);
};

#endif //