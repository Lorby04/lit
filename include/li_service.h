#ifndef __LI_TARGET__
#define __LI_TARGET__

#include <std>
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include "li_target.h"

using namespace std;
enum OP{
    ADD,
    QUERY,
    DEL,
    STOP,
    NA
}

class Service{
    std::condition_variable mCv;
    std::mutex mMutex;
    Target mTarget;
    OP mOp;

    int mThreads;

    atomic_uint64_t mQueryAttemptCount;
    atomic_uint64_t mFoundCount;

    static std::once_flag mfInst;
    static Service* mInstance = NULL;

private:    
    Service(int n);
    void start();
    static void create(int n){
        mInstance = new Service(n);
    }
    void wait(OP &aOp, Target &aTarget);
    void worker();
    string statistics() ;
public: 
    static Service *init(int n){
        std::call_once(mfInst,create);
        return mInstance;
    }
    static Service *getInstance(){
        return mInstance;
    }
public:
    void generateTargets(unsigned long n);
    void perfTest(int n);
}

#endif //