#ifndef __LI_TARGET_CACHE__
#define __LI_TARGET_CACHE__

#include <mutex>
#include <shared_mutex>
#include <vector>
#include <iostream>
#include <functional>
#include <atomic>
#include <string>
#include <map>
#include <unordered_map>
#include "atomic_mutex.h"
#include "proto_addon.h"

using namespace std;
using namespace LI;
#define TARGET_USE_MUTEX true
#if TARGET_USE_MUTEX
#define TargetSharedMutex std::shared_mutex //
#define TargetSharedLock std::shared_lock //
#define TargetUniqueLock std::unique_lock //
#else
#define TargetSharedMutex SharedMutex //std::shared_mutex //
#define TargetSharedLock ReadLock //std::shared_lock //
#define TargetUniqueLock WriteLock //std::unique_lock //
#endif

#if 0
class Target {
	string mKey;
	string mType;// type

public:
    Target(string key, string ty)
        :mKey(key), mType(ty) {}
 
    string dump() {
        string str = "{";
        str = str + mKey + ","+ mType;+"}";
        return str;
    }

    const string& key() const {
        return mKey;
    }

    const string& type() const{
        return mType;
    }
};
#endif

template<>
struct std::hash<Target>
{
    std::size_t operator()(Target const& t) const noexcept
    {
        return std::hash<std::string>{}(TypeOfTarget_Name(t.type_of_key())+t.key());
    }
};

class TargetCollectionBase{
public:    
    virtual ~TargetCollectionBase(){};
    virtual bool insert(Target &&aTarget)=0;
    virtual bool erase(const Target &aTarget)=0;
    virtual bool found(const Target &aTarget)=0;
    virtual size_t size()=0;
};

class TargetCollectionMap:public TargetCollectionBase{
public:    
    mutable TargetSharedMutex mMutex;
    std::map<Target, bool> mMap;
public:
    ~TargetCollectionMap(){};
    virtual bool insert(Target &&aTarget){
        TargetUniqueLock ul(mMutex);
        const auto [it, success] = mMap.insert({forward<Target>(aTarget),true});
        return success;
    }

    virtual bool erase(const Target &aTarget){
        TargetUniqueLock ul(mMutex);
        mMap.erase(aTarget);
        return true;
    }

    virtual bool found(const Target &aTarget){
        TargetSharedLock sl(mMutex);
        return (mMap.find(aTarget) != mMap.end());    
    }
    virtual size_t size(){
        TargetSharedLock sl(mMutex);
        return mMap.size();
    }
};

class TargetCollectionUMap:public TargetCollectionBase{
public:    
    mutable TargetSharedMutex mMutex;
    std::unordered_map<Target,bool> mMap;
public:
    TargetCollectionUMap(size_t aCapacity)
        : mMap(aCapacity){
    }
    virtual ~TargetCollectionUMap(){};
    virtual bool insert(Target &&aTarget){
        TargetUniqueLock ul(mMutex);
        const auto [it, success] = mMap.insert({forward<Target>(aTarget),true});
        return success;
    }

    virtual bool erase(const Target &aTarget){
        TargetUniqueLock ul(mMutex);
        mMap.erase(aTarget);
        return true;
    }

    virtual bool found(const Target &aTarget){
        TargetSharedLock sl(mMutex);
        return (mMap.find(aTarget) != mMap.end());    
    }
    virtual size_t size(){
        TargetSharedLock sl(mMutex);
        return mMap.size();
    }
};

class TargetCache{
    TargetCollectionBase *mCollection;

    static std::once_flag mfInst;
    static TargetCache* mInstance;

private:    
    TargetCache(string aType, size_t aCapacity){
        for (size_t i=0;i<aType.size();i++){
            aType[i] = std::toupper(aType[i]);
        }

        if (aType == "HASH"){
            cout<<"Create hash table."<< std::endl;
            mCollection = new TargetCollectionUMap(aCapacity);
        }else{
            cout<<"Create map."<< std::endl;
            mCollection = new TargetCollectionMap();        
        }        
    };

    static void create(string aType, size_t aCapacity){
        TargetCache::mInstance = new TargetCache(aType,aCapacity);
    }
    static TargetCache *getInstance(){
        return mInstance;
    }

    bool insert_(Target &&aTarget){
        return mCollection->insert(forward<Target>(aTarget));
    }

    bool erase_(const Target &aTarget){
        return mCollection->erase(aTarget);
    }

    bool found_(const Target &aTarget){
        return mCollection->found(aTarget);    
    }

    size_t size_(){
        return mCollection->size();
    }

public:
    ~TargetCache(){
        delete mCollection;
    }

    static TargetCache *init(string aType, size_t aCapacity){
        std::call_once(TargetCache::mfInst,TargetCache::create, aType,aCapacity);
        return TargetCache::getInstance();
    }

    static bool insert(Target &&aTarget){
        TargetCache::getInstance()->insert_(forward<Target>(aTarget));
        return true;
    }
    static bool found(const Target &aTarget){
        return TargetCache::getInstance()->found_(aTarget);
    }
    static bool erase(const Target &aTarget){
        return TargetCache::getInstance()->erase_(aTarget);
    }
    static size_t size(){
        return TargetCache::getInstance()->size_();
    }
};
extern bool operator==(const Target &t1, const Target &t2);


#endif
