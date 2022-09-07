#ifndef __LI_TARGET__
#define __LI_TARGET__

#include <string>
#include <map>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <iostream>
#include <functional>
#include <atomic>
#include "atomic_mutex.h"

using namespace std;
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
template<>
struct std::hash<Target>
{
    std::size_t operator()(Target const& t) const noexcept
    {
        return std::hash<std::string>{}(t.type()+t.key());
    }
};

class TargetCollectionBase{
public:    
    virtual ~TargetCollectionBase(){};
    virtual bool insert(Target &&aTarget)=0;
    virtual bool erase(Target &aTarget)=0;
    virtual bool found(Target &aTarget)=0;
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

    virtual bool erase(Target &aTarget){
        TargetUniqueLock ul(mMutex);
        mMap.erase(aTarget);
        return true;
    }

    virtual bool found(Target &aTarget){
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
    ~TargetCollectionUMap(){};
    virtual bool insert(Target &&aTarget){
        TargetUniqueLock ul(mMutex);
        const auto [it, success] = mMap.insert({forward<Target>(aTarget),true});
        return success;
    }

    virtual bool erase(Target &aTarget){
        TargetUniqueLock ul(mMutex);
        mMap.erase(aTarget);
        return true;
    }

    virtual bool found(Target &aTarget){
        TargetSharedLock sl(mMutex);
        return (mMap.find(aTarget) != mMap.end());    
    }
    virtual size_t size(){
        TargetSharedLock sl(mMutex);
        return mMap.size();
    }
};

class TargetSet{
    TargetCollectionBase *mCollection;

    std::vector<string> mTypes;
    static std::once_flag mfInst;
    static TargetSet* mInstance;

private:    
    TargetSet(string aType, size_t aCapacity){
        mTypes.push_back("From");
        mTypes.push_back("To");
        mTypes.push_back("PAI"); 
        mTypes.push_back("Location");

        for (int i=0;i<aType.size();i++){
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
        TargetSet::mInstance = new TargetSet(aType,aCapacity);
    }
    static TargetSet *getInstance(){
        return mInstance;
    }

    bool insert_(Target &&aTarget){
        return mCollection->insert(forward<Target>(aTarget));
    }

    bool erase_(Target &aTarget){
        return mCollection->erase(aTarget);
    }

    bool found_(Target &aTarget){
        return mCollection->found(aTarget);    
    }

    size_t size_(){
        return mCollection->size();
    }

public:
    ~TargetSet(){
        delete mCollection;
    }
    static std::vector<string> &types() { return TargetSet::getInstance()->mTypes; }
    static TargetSet *init(string aType, size_t aCapacity){
        std::call_once(TargetSet::mfInst,TargetSet::create, aType,aCapacity);
        return TargetSet::getInstance();
    }

    static bool insert(Target &&aTarget){
        TargetSet::getInstance()->insert_(forward<Target>(aTarget));
        return true;
    }
    static bool found(Target &aTarget){
        return TargetSet::getInstance()->found_(aTarget);
    }
    static bool erase(Target &aTarget){
        return TargetSet::getInstance()->erase_(aTarget);
    }
    static size_t size(){
        return TargetSet::getInstance()->size_();
    }
};
extern bool operator<(const Target &t1, const Target &t2);
extern bool operator==(const Target &t1, const Target &t2);


#endif
