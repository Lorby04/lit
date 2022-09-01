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

const size_t SemaphoreLimit = 1000;
template <class T>
class Channel{
public:    
    typedef std::unique_ptr<T> PT;
private:
//    std::condition_variable mCv;
    std::counting_semaphore<SemaphoreLimit> mCtSemaphore;
    std::mutex mMutex;
    std::binary_semaphore mSemaphore;
    std::list<PT> mQ;
    bool mClosed;
    bool mShutdown;
    size_t mSize;

    atomic_uint64_t mSentRequestCount;
    atomic_uint64_t mPushedCount;
    atomic_uint64_t mPopedCount;
private:
    Channel(size_t size):
        mCtSemaphore(0),
        mSemaphore(1){
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
    bool notify = false;
    for(;;){
//        cout <<  "Waiting for lock to receive:"<< dump() << endl;
//        std::unique_lock lk(mMutex);
//        cout <<  "Waiting for condition to receive:"<< dump() << endl;
//        mCv.wait(lk);
//        cout <<  "Checking to receive:"<< dump() << endl;
        mCtSemaphore.acquire();
        std::unique_lock lk(mMutex);
        if (mShutdown){
//            mCv.notify_all();
            mCtSemaphore.release(1);
            throw ShutdownException();
        }
        if (mQ.empty()){
            if (mClosed){
//                mCv.notify_all();
                mCtSemaphore.release(1);
                throw CloseException();
            }else{
                continue;
            }
        }else{
            ptr = std::move(mQ.front());
//            cout <<  "Size before taking:"<< mQ.size() << endl;

            mQ.pop_front();
//            cout <<  "Size after taking:"<< mQ.size() << endl;
            mPopedCount++;
            break;
        }
    }
//    mCv.notify_one();
//    mCtSemaphore.release();
    mSemaphore.release();
    return ptr;
}
template <class T>
bool Channel<T>::send(Channel<T>::PT aT){
    bool sent = false;
    mSentRequestCount++;
    for(;!mClosed;){
        {
            std::unique_lock lk(mMutex);
//            cout <<  "Checking to send:"<< dump() << endl;
            if (mQ.size() < mSize){
                mQ.push_back(std::move(aT));
                mPushedCount++;
                sent = true;
            }else{//else, full, waiting for receiver to take
                assert(mQ.size() >= mSize);
            }
        }

//        cout <<sent << ": End of loop send:"<< dump() << endl;
//        mCv.notify_one();

        if (sent){
            mCtSemaphore.release();
            break;
        }else{
            mSemaphore.acquire();
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
//    mCv.notify_all();
    mCtSemaphore.release(SemaphoreLimit);
}
template <class T>
void Channel<T>::shutdown(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
        mShutdown = true;
    }
//    mCv.notify_all();
    mCtSemaphore.release(SemaphoreLimit);
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