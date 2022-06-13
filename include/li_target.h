#ifndef __LI_TARGET__
#define __LI_TARGET__

#include <string>
#include <map>

using namespace std;
class LiTarget{
public:
    LiTarget(){}
    LiTarget(string &key, string &type)
        :mKey(key), mTargetType(type){
            
        }
    void setTarget(string &key, string &type){
        mKey = key;
        mTargetType = type;
    }
    const string& getKey() const{return mKey;}
    const string& getTargetType() const{return mTargetType;}
private:
    string mKey;
    string mTargetType;
};

typedef map<LiTarget, bool> LiTargetSet;
extern bool operator<(const LiTarget &t1, const LiTarget &t2);
#endif
