#include "li_target.h"

bool operator<(const LiTarget &t1, const LiTarget &t2){
    if (t1.getTargetType() < t2.getTargetType()) return true;
    if (t1.getTargetType() > t2.getTargetType()) return false;
    return t1.getKey()<t2.getKey();
}