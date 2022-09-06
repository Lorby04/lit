#include "li_target.h"
TargetSet* TargetSet::mInstance = nullptr;
std::once_flag TargetSet::mfInst;
bool operator<(const Target &t1, const Target &t2){
    if (t1.type() < t2.type()) return true;
    if (t1.type() > t2.type()) return false;
    return t1.key() < t2.key();
}

bool operator==(const Target &t1, const Target &t2){
    return ((t1.type() == t2.type()) && (t1.key() == t2.key()));
}
