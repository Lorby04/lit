#include "li_target.h"

bool operator<(const Target &t1, const Target &t2){
    if (t1.type() < t2.type()) return true;
    if (t1.type() > t2.type()) return false;
    return t1.key() < t2.key();
}