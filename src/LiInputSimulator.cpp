#include <utility>
#include <iostream>
#include "li_lib.h"

using namespace std;
unsigned long totalEntries = 2000000;
void generateTargets(){
    unsigned long digit = 1000000001;
    string types[] = {"From","To", "PAI", "Location"};
    int numOfTypes = sizeof(types)/sizeof(string);

    cout << "Total types: "<<numOfTypes << ",Start from : " << digit << endl;
    for (int i = 0; i<totalEntries;i++, digit++){
        if(digit%10 == 0){
            digit++;
        }
        string key = to_string(digit);
        string type = types[i%numOfTypes];
        LiInstance::getInstance()->liLib().insert(LiTarget(key,type));
        if(i%(totalEntries/100) == 0){
            cout << i << " entries added" << endl;
        }
    }
    cout << "End at :" << digit << endl;
}