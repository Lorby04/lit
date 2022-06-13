#ifndef __LI_LIB__
#define __LI_LIB__

#include <utility>
#include <stdexcept>
#include "li_target.h"

using namespace std;
class LiLib{
public:
    static LiLib* getInstance() { 
        if (mInstance == nullptr){
            mInstance = new LiLib();
            if (mInstance==nullptr){
                throw runtime_error("Failed to create LiLib instance");
            }
        }
        return mInstance; 
    }

    LiTargetSet& getTargets(){return mTargets;}
    bool matches(LiTarget &aTarget){
        mChecked++;
        auto it = mTargets.find(aTarget);
        if(it != mTargets.end()){
            mMatched++;
            return true;
        }
        return false;
    }

    bool insert(LiTarget &&aTarget){
        const auto [it, success] = mTargets.insert({forward<LiTarget>(aTarget),true});
        return success;
    }

    void erase(LiTarget &aTarget){
        mTargets.erase(aTarget);
    }

private:
    LiTargetSet mTargets;

private:
    int mChecked;
    int mMatched;

private:
    static LiLib *mInstance;

private:
    LiLib()
        :mChecked(0),mMatched(0){
    }
};

class LiInstance{
public:
    static LiInstance* getInstance() { 
        if(mInstance == nullptr){
            mInstance = new LiInstance();
            if (mInstance==nullptr){
                throw runtime_error("Failed to create LI instance");
            }
        }
        return mInstance; 
    }

    LiLib& liLib(){return *mLib;}
private:
    LiLib *mLib; 

private:
    static LiInstance *mInstance;

private:
    LiInstance(){
        mLib = LiLib::getInstance();
    }
};

extern void generateTargets();
extern void perfTest(int n, int tid);
#endif