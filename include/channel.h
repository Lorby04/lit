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
#include "atomic_mutex.h"

#define CHAN_USE_MUTEX false
#define CHAN_READ_AS_WRITE true

#if CHAN_USE_MUTEX

#if CHAN_READ_AS_WRITE
#define ChanSharedMutex std::mutex //SharedMutex //
#define ChanSharedLock std::unique_lock //ReadLock //
#else
#define ChanSharedMutex std::shared_mutex //SharedMutex //
#define ChanSharedLock std::shared_lock //ReadLock //
#endif

#define ChanUniqueLock std::unique_lock //WriteLock //

#else //#define ChanShared



#if CHAN_READ_AS_WRITE
#define ChanSharedMutex Mutex //
#define ChanSharedLock WriteLock //
#else
#define ChanSharedMutex SharedMutex //
#define ChanSharedLock ReadLock //
#endif

#define ChanUniqueLock WriteLock //
#endif


const size_t defaultSize = 1;
class ShutdownException{};
class CloseException{};

const size_t SemaphoreLimitAccess = 10;
const size_t BatchNotification = 10;
const uint64_t MaxCapacityBitWidth = 16;

template <class T>
class Channel{
public:    
    typedef std::unique_ptr<T> PT;    
public:
    virtual ~Channel(){}
    virtual uint64_t size() const=0;
    virtual PT receive() = 0;
    virtual bool send(PT aT)=0;
    virtual void close()=0;
    virtual void shutdown()=0;
    virtual string dump()=0;
};

typedef bool (*Predicate)();
class AtomicCV{
private:
    atomic_uint_fast32_t mCv;
public:  
    template<class LOCK, class Predicate>
    void wait( LOCK& lock,  Predicate stop_waiting ){
        while(!stop_waiting()){
            lock.unlock();
            mCv.wait(mCv.load());
            lock.lock();
        }
    }

    void notify_one(){
        mCv++;
        mCv.notify_one();
    }

    void notify_all(){
        mCv++;
        mCv.notify_all();
    }
};

template <class T>
class RBChannel:public Channel<T>{
public:    
    typedef std::unique_ptr<T> PT;
private:
//    std::condition_variable_any mQAccess;
    AtomicCV mQAccess;
//    std::counting_semaphore<SemaphoreLimitAccess> mQAccess;
    mutable ChanSharedMutex mMutex;

    PT *mQ;
    atomic_uint64_t mRIndex; // index of entry that can be read unless it is the same as mWIndex
    uint64_t mWIndex; // index of entry that being written
    
    mutable std::binary_semaphore mReadyToInQ;
    bool mClosed;
    bool mShutdown;
    uint64_t mSize;
    uint64_t mMask;

    atomic_uint64_t mSentRequestCount;
    atomic_uint64_t mPushedCount;
    atomic_uint64_t mPopedCount;
    atomic_uint64_t mBlockSenderCount;
private:
    RBChannel(uint64_t aBitWidth):
        mRIndex(0),
        mWIndex(0),
        mReadyToInQ(1),
        mSentRequestCount(0),
        mPushedCount(0),
        mPopedCount(0),
        mBlockSenderCount(0)
    {
        if (aBitWidth > MaxCapacityBitWidth){
            aBitWidth = MaxCapacityBitWidth;
        }
        mSize = (1 << aBitWidth);
        mMask = mSize-1;
        mQ = new PT[mSize];
        mClosed = false;
        mShutdown = false;
    }
    RBChannel (const RBChannel&) = delete;
    RBChannel& operator=(const RBChannel&) = delete;

private:
    bool full(uint64_t aWIndex, uint64_t aRIndex){
       if (!((aWIndex>=aRIndex) && (aWIndex<=aRIndex+mSize))){
            cout<<"aW:" << aWIndex<<", aR "<<aRIndex<< ", Size: " << mSize << std::endl;
            cout <<"Stats:"<< dump() << std::endl;
            assert(false);
       }
        return aWIndex - aRIndex==mSize;
    }
    bool empty(uint64_t aWIndex, uint64_t aRIndex){
        return (aWIndex<=aRIndex);
    }
    uint64_t len(uint64_t aWIndex, uint64_t aRIndex)const{
        uint64_t ln = aWIndex-aRIndex;
        assert(ln<=mSize);
        //cout <<"ln:" << ln <<"="<<aWIndex<<"-"<<aRIndex<<endl;
        return ln;
    }
    uint64_t index(uint64_t aIndex){return aIndex & mMask;}
public:
    static RBChannel* create(uint64_t aBitWidth){
        return new RBChannel(aBitWidth);
    }
    ~RBChannel(){
        delete mQ;
    }

    uint64_t size() const{
        ChanSharedLock lk(mMutex);
        return len(mWIndex, mRIndex.load());
    }

    PT receive();
    bool send(PT aT);
    void close();
    void shutdown();
    string dump();
};

template <class T>
RBChannel<T>::PT RBChannel<T>::receive(){
    PT ptr;
    auto checkAndTake=[&]()->bool{
        if (mShutdown){
            mQAccess.notify_all();
            throw ShutdownException();
        }
        uint64_t rindex = mRIndex++;
        //cout <<"R:rindex " << rindex << ", RIndex 1:"<<mRIndex.load() <<", WIndex: "<<mWIndex <<endl;
        if (rindex < mWIndex){ // readable, keep mRIndex with new value (+1)
            ptr = std::move(mQ[index(rindex)]);
            mPopedCount++;
            return true;
        }
        
        //cout <<"R:rindex " << rindex << ", RIndex 2:"<<mRIndex.load() <<", WIndex: "<<mWIndex <<endl;
        --mRIndex; // Since the current procedure didn't read anything, rollback the change
                   // Considering multiple threads may reach to the situation simultaneously
                   // At this point, the mRIndex may have been rolled back multiple times
                   // To reduce the chance of leaving unhandled data, needs to reload to check again
        //cout <<"R:rindex " << rindex << ", RIndex 3:"<<mRIndex.load() <<", WIndex: "<<mWIndex <<endl;
        if (mClosed && empty(mWIndex, mRIndex.load())){
            mQAccess.notify_all();
            throw CloseException();
        }
        return false;
    };
    ChanSharedLock lk(mMutex);
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
bool RBChannel<T>::send(RBChannel<T>::PT aT){
    mSentRequestCount++;
    for(;!mClosed;){
        {
            ChanUniqueLock lk(mMutex);
            uint64_t rindex = mRIndex.load();
            //cout <<"W:rindex " << rindex << ", RIndex 5:"<<mRIndex.load() << ", WIndex: "<<mWIndex <<endl;

            if (!full(mWIndex,rindex)){
                mQ[index(mWIndex++)]=std::move(aT); // Add to queue, move index to next available one
                if (len(mWIndex,rindex) >= BatchNotification){
                    mQAccess.notify_all();
                }else{
                    mQAccess.notify_one();
                }
                mPushedCount++;
                return true;
            }else{//else, full, waiting for receiver to take
                assert(len(mWIndex,rindex) == mSize);
                /*{
                    cout<<"W:" << mWIndex<<", R: "<<rindex << "len:" << len(mWIndex,rindex)<< ", Size: " << mSize << std::endl;
                    cout <<"Stats:"<< dump() << std::endl;
                    assert(false);
                }*/
                mBlockSenderCount++;
            }
        }
        mReadyToInQ.acquire();
    }
    return false;
}
template <class T>
void RBChannel<T>::close(){
    {
        cout <<"Prepare to close the channel" << endl;
        ChanUniqueLock lk(mMutex);
        mClosed = true;
        //cout << "Close RBChannel" << std::endl;
    }
    cout <<"The channel is closed" << endl;
    mQAccess.notify_all();
}
template <class T>
void RBChannel<T>::shutdown(){
    {
        cout <<"The channel is being shutdown" << endl;
        ChanUniqueLock lk(mMutex);
        mClosed = true;
        mShutdown = true;
    }
    cout <<"The channel is shutdown" << endl;
    mQAccess.notify_all();
}

template <class T>
string RBChannel<T>::dump()  {
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

template <class T>
class LTChannel:public Channel<T>{
public:    
    typedef std::unique_ptr<T> PT;
private:
//  std::condition_variable 
    AtomicCV mQAccess;
//    std::counting_semaphore<SemaphoreLimitAccess> mQAccess;
    mutable std::mutex mMutex;
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
    LTChannel(uint64_t aBitWidth):
        mReadyToInQ(1),
        mSentRequestCount(0),
        mPushedCount(0),
        mPopedCount(0),
        mBlockSenderCount(0)
    {
        if (aBitWidth > MaxCapacityBitWidth){
            aBitWidth = MaxCapacityBitWidth;
        }
        mSize = (1 << aBitWidth);
        mClosed = false;
        mShutdown = false;
    }
    LTChannel (const LTChannel&) = delete;
    LTChannel& operator=(const LTChannel&) = delete;
public:
    ~LTChannel(){};
    static LTChannel* create(uint64_t aBitWidth){
        return new LTChannel(aBitWidth);
    }
    uint64_t size() const{
        std::unique_lock lk(mMutex);
        return mQ.size();
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
template <class T>
static Channel<T>* createChannel(string aType, uint64_t aBitWidth){
    for (int i=0;i<aType.size();i++){
        aType[i] = std::toupper(aType[i]);
    }

    if (aType == "LIST"){
        cout<<"Create list based channel."<< std::endl;
        return LTChannel<T>::create(aBitWidth);
    }else{
        cout<<"Create ring buffer based channel."<< std::endl;
        return RBChannel<T>::create(aBitWidth);
    }
}