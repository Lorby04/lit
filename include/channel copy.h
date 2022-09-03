#ifndef __CHANNEL__
#define __CHANNEL__

#include <string>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <semaphore>
#include <iostream>
#include <memory>
#include <list>
#include <cassert>
#include <atomic>

const size_t defaultSize = 1;
class ShutdownException{};
class CloseException{};

const size_t SemaphoreLimitAccess = 10;
const size_t BatchNotification = 10;

template <class T>
class LTChannel:public Channel<T>{
public:    
    typedef std::unique_ptr<T> PT;
private:
    std::condition_variable mQAccess;
//    std::counting_semaphore<SemaphoreLimitAccess> mQAccess;
    std::mutex mMutex;
    std::binary_semaphore mReadyToInQ;
    std::list<PT> mQ;
    bool mClosed;
    bool mShutdown;
    size_t mSize;

    atomic_uint64_t mSentRequestCount;
    atomic_uint64_t mPushedCount;
    atomic_uint64_t mPopedCount;
    atomic_uint64_t mBlockSenderCount;
private:
    LTChannel(size_t size):
        mReadyToInQ(1),
        mSentRequestCount(0),
        mPushedCount(0),
        mPopedCount(0),
        mBlockSenderCount(0)
    {
        if (size == 0){
            mSize = defaultSize;
        }else{
            mSize = size;
        }
        mClosed = false;
        mShutdown = false;
    }
    LTChannel (const LTChannel&) = delete;
    LTChannel& operator=(const LTChannel&) = delete;
public:
    static LTChannel* create(size_t size){
        return new LTChannel(size);
    }

    PT receive();
    bool send(PT aT);
    void close();
    void shutdown();
    string dump();
};

template <class T>
LTChannel<T>::PT LTChannel<T>::receive(){
    PT ptr;
    auto checkAndTake=[&]()->bool{
        if (mShutdown){
            mQAccess.notify_all();
            throw ShutdownException();
        }
        if(!mQ.empty()){
            ptr = std::move(mQ.front());
                mQ.pop_front();
                mPopedCount++;
            return true;
        }
        
        assert(mQ.empty());
        if (mClosed){
            mQAccess.notify_all();
            throw CloseException();
        }
        return false;
    };
    std::unique_lock lk(mMutex);
    mQAccess.wait(lk,checkAndTake);
/*
    for(;;){
        std::unique_lock lk(mMutex);
        if  (checkAndTake()){
            break;
        }
        mQAccess.wait(lk);
        if  (checkAndTake()){
            break;
        }
    }
*/
    mReadyToInQ.release();
    return ptr;
}
template <class T>
bool LTChannel<T>::send(LTChannel<T>::PT aT){
    mSentRequestCount++;
    for(;!mClosed;){
        {
            std::unique_lock lk(mMutex);
            if (mQ.size() < mSize){
                mQ.push_back(std::move(aT));
                if (mQ.size() >= BatchNotification){
                    mQAccess.notify_all();
                }else{
                    mQAccess.notify_one();
                }
                mPushedCount++;
                return true;
            }else{//else, full, waiting for receiver to take
                assert(mQ.size() >= mSize);
                mBlockSenderCount++;
            }
        }
        mReadyToInQ.acquire();
    }
    return false;
}
template <class T>
void LTChannel<T>::close(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
    }
    mQAccess.notify_all();
}
template <class T>
void LTChannel<T>::shutdown(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
        mShutdown = true;
    }
    mQAccess.notify_all();
}

template <class T>
string LTChannel<T>::dump()  {
    string str = "Sent Request:"
        + to_string(mSentRequestCount.load(std::memory_order_seq_cst))
        + ", Pushed:"
        + to_string(mPushedCount.load(std::memory_order_seq_cst))
        + ", Poped "
        + to_string(mPopedCount.load(std::memory_order_seq_cst))
        + ", Sender blocked: "
        + to_string(mBlockSenderCount.load(std::memory_order_seq_cst))
        + ".";

    return str;
}
#endif //