#include <std>
#include <string>
#include <mutex>
#include <semaphore>
#include <condition_variable>
#include <iostream>
#include <memory>

const size_t defaultSize = 1;
class ShutdownException{}
class CloseException{}

template <class T>
class Channel{
    typedef std::unique_ptr<T> PT

    std::condition_variable mCv;
    std::mutex mMutex;
    std::binary_semaphore mSemaphore;
    std::list<PT> mQ;
    bool mClosed;
    bool mShutdown;
    size_t mSize;

private:
    Channel(size_t size){
        if size == 0{
            mSize = defaultSize;
        }else{
            mSize = size;
        }
        mClosed = false;
        mShutdown = false;
    }
    delete operator=;
public:
    static Channel* create(size_t size){
        return new Channel(size);
    }

    PT receive(){
        PT ptr;
        bool notify = false;
        for(;;){
            std::unique_lock lk(mMutex);
            mCv.wait(lk);
            if mShutdown{
                 mCv.notify_all();
                throw ShutdownException();
            }
            if mQ.empty(){
                if mClosed{
                    mCv.notify_all();
                    throw CloseException();
                }else{
                    continue;
                }
            }else{
                ptr = mQ.front();
                mQ.pop_front();
                notify = !mQ.empty();
                break;
            }
        }
        if notify{
            mCv.notify_one();
        }
        mSemaphore.releases();
        return ptr;
    }
    bool send(PT aT){
        bool sent = false;
        for(;!mClosed;){
            {
                std::unique_lock lk(mMutex);
                if mQ.size() < mSize{
                    mQ.push_back(aT);
                    sent = true;
                }
            }
            if sent{
                mCv.notify_one();
                return sent;
            }else{
                mSemaphore.acquire();
            }
        }
        return sent;
    }
    void close(){
        {
            std::unique_lock lk(mMutex);
            mClosed = true;
        }
        mCv.notify_all();
    }
    void shutdown(){
        {
            std::unique_lock lk(mMutex);
            mClosed = true;
            mShutdown = true;
        }
        mCv.notify_all();
    }
}