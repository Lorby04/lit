#ifndef __PROTO_ADDON__
#define __PROTO_ADDON__

#include "li.pb.h"

namespace LI{
inline bool operator<(const Target &t1, const Target &t2){
    if (t1.type_of_key() < t2.type_of_key()) return true;
    if (t1.type_of_key() > t2.type_of_key()) return false;
    return t1.key() < t2.key();
}

inline bool operator==(const Target &t1, const Target &t2){
    return ((t1.type_of_key() == t2.type_of_key()) && (t1.key() == t2.key()));
}
}
#endif