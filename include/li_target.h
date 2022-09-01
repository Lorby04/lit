#ifndef __LI_TARGET__
#define __LI_TARGET__

#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <iostream>

using namespace std;

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

class TargetSet{
    mutable std::shared_mutex mMutex;
    std::map<Target, bool> mMap;

    std::vector<string> mTypes;
    static std::once_flag mfInst;
    static TargetSet* mInstance;

private:    
    TargetSet(){
        mTypes.push_back("From");
        mTypes.push_back("To");
        mTypes.push_back("PAI"); 
        mTypes.push_back("Location");
    };
    static void create(){
        TargetSet::mInstance = new TargetSet();
    }
    static TargetSet *getInstance(){
        std::call_once(TargetSet::mfInst,TargetSet::create);
        return mInstance;
    }

    bool insert_(Target &&aTarget){
        unique_lock ul(mMutex);
        const auto [it, success] = mMap.insert({forward<Target>(aTarget),true});
        return success;
    }

    bool erase_(Target &aTarget){
        unique_lock ul(mMutex);
        mMap.erase(aTarget);
        return true;
    }

    bool found_(Target &aTarget){
        shared_lock sl(mMutex);
        return (mMap.find(aTarget) != mMap.end());    
    }
    size_t size_(){
        shared_lock sl(mMutex);
        return mMap.size();
    }

public:
    static std::vector<string> &types() { return TargetSet::getInstance()->mTypes; }
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
#endif
