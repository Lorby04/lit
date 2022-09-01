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
class Channel{
public:    
    typedef std::unique_ptr<T> PT;
private:
//    std::condition_variable mCv;
    std::counting_semaphore<SemaphoreLimitAccess> mQAccess;
    std::mutex mMutex;
    std::binary_semaphore mReadyToInQ;
    std::list<PT> mQ;
    bool mClosed;
    bool mShutdown;
    size_t mSize;

    atomic_uint64_t mSentRequestCount;
    atomic_uint64_t mPushedCount;
    atomic_uint64_t mPopedCount;
private:
    Channel(size_t size):
        mQAccess(0),
        mReadyToInQ(1){
        if (size == 0){
            mSize = defaultSize;
        }else{
            mSize = size;
        }
        mClosed = false;
        mShutdown = false;
    }
    Channel (const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
public:
    static Channel* create(size_t size){
        return new Channel(size);
    }

    PT receive();
    bool send(PT aT);
    void close();
    void shutdown();
    string dump();
};

template <class T>
Channel<T>::PT Channel<T>::receive(){
    PT ptr;
    for(;;){
        mQAccess.acquire();
        std::unique_lock lk(mMutex);
        if (mShutdown){
            mQAccess.release(BatchNotification);
            throw ShutdownException();
        }
        if (mQ.empty()){
            if (mClosed){
                mQAccess.release(BatchNotification);
                throw CloseException();
            }else{
                continue;
            }
        }else{
            ptr = std::move(mQ.front());
            mQ.pop_front();
            mPopedCount++;
            break;
        }
    }
    mReadyToInQ.release();
    return ptr;
}
template <class T>
bool Channel<T>::send(Channel<T>::PT aT){
    bool sent = false;
    mSentRequestCount++;
    for(;!mClosed;){
        {
            std::unique_lock lk(mMutex);
            if (mQ.size() < mSize){
                mQ.push_back(std::move(aT));
                mPushedCount++;
                sent = true;
            }else{//else, full, waiting for receiver to take
                assert(mQ.size() >= mSize);
            }
        }
        if (sent){
            mQAccess.release(1);
            return sent;
        }else{
            mReadyToInQ.acquire();
        }
    }
    return sent;
}
template <class T>
void Channel<T>::close(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
    }
    mQAccess.release(BatchNotification);
}
template <class T>
void Channel<T>::shutdown(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
        mShutdown = true;
    }
    mQAccess.release(BatchNotification);
}

template <class T>
string Channel<T>::dump()  {
    string str = "Sent Request:"
        + to_string(mSentRequestCount.load(std::memory_order_seq_cst))
        + ", Pushed:"
        + to_string(mPushedCount.load(std::memory_order_seq_cst))
        + ", Poped "
        + to_string(mPopedCount.load(std::memory_order_seq_cst))
        + ".";

    return str;
}
#endif //