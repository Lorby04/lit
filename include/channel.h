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
const uint64_t MaxCapacityBitWidth = 16;

template <class T>
class Channel{
public:    
    typedef std::unique_ptr<T> PT;
private:
    std::condition_variable_any mQAccess;
//    std::counting_semaphore<SemaphoreLimitAccess> mQAccess;

    std::shared_mutex mMutex;

    PT *mQ;
    atomic_uint64_t mRIndex; // index of entry that can be read unless it is the same as mWIndex
    uint64_t mWIndex; // index of entry that being written
    
    std::binary_semaphore mReadyToInQ;
    bool mClosed;
    bool mShutdown;
    uint64_t mSize;
    uint64_t mMask;

    atomic_uint64_t mSentRequestCount;
    atomic_uint64_t mPushedCount;
    atomic_uint64_t mPopedCount;
    atomic_uint64_t mBlockSenderCount;
private:
    Channel(int aBitWidth):
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
    Channel (const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

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
    uint64_t len(uint64_t aWIndex, uint64_t aRIndex){
        uint64_t ln = aWIndex-aRIndex;
        assert(ln<=mSize);
        //cout <<"ln:" << ln <<"="<<aWIndex<<"-"<<aRIndex<<endl;
        return ln;
    }
    uint64_t index(uint64_t aIndex){return aIndex & mMask;}
public:
    static Channel* create(uint64_t size){
        return new Channel(size);
    }
    ~Channel(){
        delete mQ;
    }

    uint64_t size() const{
        std::shared_lock lk(mMutex);
        return len(mWIndex, mRIndex.load());
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
    std::shared_lock lk(mMutex);
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
bool Channel<T>::send(Channel<T>::PT aT){
    mSentRequestCount++;
    for(;!mClosed;){
        {
            std::unique_lock lk(mMutex);
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
void Channel<T>::close(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
        //cout << "Close Channel" << std::endl;
    }
    mQAccess.notify_all();
}
template <class T>
void Channel<T>::shutdown(){
    {
        std::unique_lock lk(mMutex);
        mClosed = true;
        mShutdown = true;
    }
    mQAccess.notify_all();
}

template <class T>
string Channel<T>::dump()  {
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