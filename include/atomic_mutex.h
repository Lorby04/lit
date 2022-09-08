#ifndef __ATOMIC_MUTEX__
#define __ATOMIC_MUTEX__

#include <string>
#include <thread>
#include <iostream>
#include <memory>
#include <cassert>
#include <atomic>

using namespace std;
#define SMX_UNLOCKED  0
class SharedMutex{
private:    
    atomic_uint_fast64_t mR;
    atomic_uint_fast64_t mW;
    atomic_uint_fast64_t mWCount;
private:
    uint_fast64_t getLocked(){ 
        auto count = mWCount.load();
        if (++count == 0){
            ++count;
        }
        mWCount.store(count);
        return count;
    }
public:
    SharedMutex():
        mR(0),
        mW(0),
        mWCount(0)
//        smx_unlocked(0),
//        smx_locked(1)
        {}

    void wLock(){
        uint_fast64_t smx_unlocked = SMX_UNLOCKED;
        uint_fast64_t smx_locked = getLocked();

        while(!mW.compare_exchange_strong(smx_unlocked, smx_locked)) {
            // If mW is changed from locked to unlocked, wait returns immediately
            smx_unlocked = SMX_UNLOCKED; // It was changed in the exchange operation
            mW.wait(smx_locked);
        }
        assert(mW.load() == smx_locked);
        // Now wlock is completed, waiting for cleaning up rlock
        for (auto rlocked = mR.load();rlocked != 0;rlocked = mR.load()){
            mR.wait(rlocked);
        }
    }
    void wUnlock(){
        uint_fast64_t smx_unlocked = SMX_UNLOCKED;
        assert(mW.load() != smx_unlocked);
        //{
        //    cout << "mW after exchange:"<<mW.load() <<",smx_locked: "<<smx_locked << ",smx_unlocked: "<<smx_unlocked <<endl;
        //}
        mW.store(smx_unlocked);
        mW.notify_one();
    }

    void rLock(){
        uint_fast64_t smx_unlocked = SMX_UNLOCKED;
        uint_fast64_t smx_locked = getLocked();
        // Lock writing operation first
        while(!mW.compare_exchange_strong(smx_unlocked, smx_locked)) {
            // If mW is changed from locked to unlocked, wait returns immediately
            smx_unlocked = SMX_UNLOCKED;
            mW.wait(smx_locked);
        }
        assert(mW.load() == smx_locked);
        //{
        //    cout << "mW after exchange:"<<mW.load() <<",smx_locked: "<<smx_locked << ",smx_unlocked: "<<smx_unlocked <<endl;
        //}
        assert(mR.load() >= 0);
        mR.fetch_add(1);
        mW.store(SMX_UNLOCKED); // Unlock writing restriction
    }
    void rUnlock(){
        assert(mR.load() >= 1);
        mR.fetch_sub(1);
        mR.notify_all();
    }
};

class Mutex{
private:    
    atomic_uint_fast64_t mW;
    atomic_uint_fast64_t mWCount;
private:
    uint_fast64_t getLocked(){ 
        auto count = mWCount.load();
        if (++count == 0){
            ++count;
        }
        mWCount.store(count);
        return count;
    }
public:
    Mutex():
        mW(0),
        mWCount(0)
//        smx_unlocked(0),
//        smx_locked(1)
        {}

    void wLock(){
        uint_fast64_t smx_unlocked = SMX_UNLOCKED;
        uint_fast64_t smx_locked = getLocked();

        while(!mW.compare_exchange_strong(smx_unlocked, smx_locked)) {
            // If mW is changed from locked to unlocked, wait returns immediately
            smx_unlocked = SMX_UNLOCKED; // It was changed in the exchange operation
            mW.wait(smx_locked);
        }
        assert(mW.load() == smx_locked);
    }
    void wUnlock(){
        uint_fast64_t smx_unlocked = SMX_UNLOCKED;
        assert(mW.load() != smx_unlocked);
        //{
        //    cout << "mW after exchange:"<<mW.load() <<",smx_locked: "<<smx_locked << ",smx_unlocked: "<<smx_unlocked <<endl;
        //}
        mW.store(smx_unlocked);
        mW.notify_one();
    }

    void rLock(){
        wLock();
    }
    void rUnlock(){
        wUnlock();
    }
};


class ReadLock{
    SharedMutex &mMutex;
public:
    ReadLock(SharedMutex &mux)
        :mMutex(mux)
    {
        lock();
    }
    ~ReadLock()
    {
        unlock();
    }
public:
    void lock() { mMutex.rLock();}
    void unlock() { mMutex.rUnlock();}
};

template <class MTX>
//concept Lock=requires(MTX mtx){
//    mtx.wLock();
//    mtx.wUnlock();
//};
class WriteLock{
    MTX &mMutex;
public:
    WriteLock(MTX &mux)
        :mMutex(mux)
    {
        lock();
    }
    ~WriteLock()
    {
        unlock();
    }
public:
    void lock() { mMutex.wLock();}
    void unlock() { mMutex.wUnlock();}
};
#endif