#ifndef __LI_TARGET__
#define __LI_TARGET__

#include <std>
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>

using namespace std;

class Target {
	string mKey;
	string mType;// type

public:
    Target(string key, string ty)
        :mKey(key), mType(ty) {}
    static Target *newTarget(key, ty string) {
        return new Target(key, ty)
    }
    string dump() {
        string str = "{"; strings.Builder
        str = str + me.key + ","+me.ty+"}";
        return str
    }

    string& key() {
        return mKey;
    }

    string& type(){
        return mType;
    }
    const static string mTypes[] = {"From","To", "PAI", "Location"};
}


class TargetSet{
    mutable std::shared_mutex mMutex;
    std::map<Target, bool> mMap;

    static std::once_flag mfInst;
    static TargetSet* mInstance = NULL;

private:    
    TargetSet()= default;
    static void create(){
        mInstance = new TargetSet();
    }
    static TargetSet *getOrCreate(){
        std::call_once(mfInst,create)
        return mInstance;
    }

    bool insert(Target *aTarget){
        unique_lock ul(mMutex);
        const auto [it, success] = mMap.insert({forward<Target>(*aTarget),true});
        return success;
    }

    bool erase(Target *aTarget){
        unique_lock ul(mMutex);
        mMap.erase(*aTarget);
    }

    bool found(Target *aTarget){
        shared_lock sl(mMutex);
        return (mMap.find(*aTarget) != mMap.end());    
    }
    size_t size(){
        shared_lock sl(mMutex);
        return (mMap.size());
    }

public:
    static bool insert(Target *aTarget){
        TargetSet::getOrCreate()->insert(aTarget);
    }
    static bool found(Target *aTarget){
        TargetSet::getOrCreate()->found(aTarget);
    }
    static bool erase(Target *aTarget){
        TargetSet::getOrCreate()->erase(aTarget);
    }
    static bool size(){
        TargetSet::getOrCreate()->size();
    }
}
extern bool operator<(const Target &t1, const Target &t2);
#endif
