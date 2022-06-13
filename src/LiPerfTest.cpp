#include <iostream>
#include "li_lib.h"

extern unsigned long totalEntries;
void perfTest(int n, int tid){
#define TID "tid["<<tid<<"]"<<
    unsigned long low  = 1000000001;
    unsigned long high = 9999999999;
    string types[] = {"From","To", "PAI", "Location"};
    int numOfTypes = sizeof(types)/sizeof(string);

    auto full_start = clock();
    cout << TID "Test starts from : " << low << " at: " << full_start << endl;
    for (int round = 0; round < n; round++){
        auto start = clock();
        cout << TID "Round " << round+1 << " starts : " << low << " at: " << full_start << endl;
        for (int i = 0; i<totalEntries;i++){
            if(low%10 == 0){
                low++;
            }
            string lKey = to_string(low);
            string lType = types[i%numOfTypes];
            LiTarget llt(lKey,lType);
            LiInstance::getInstance()->liLib().matches(llt);
            low++;

            if(high%10 == 0){
                high--;
            }
            string hKey = to_string(high);
            string hType = types[i%numOfTypes];
            LiTarget hlt(hKey,hType);
            LiInstance::getInstance()->liLib().matches(hlt);
            high--;
        }
        auto end = clock();
        cout << TID "Round "<<round+1<<" ends :"<< end << ". Time used:" << double(end-start)/CLOCKS_PER_SEC<<" seconds" << endl;
    }
    auto full_end = clock();
    cout << TID "Test ends at : " << full_end << ". Time used: " << double(full_end-full_start)/CLOCKS_PER_SEC 
        << " seconds for " << n << "rounds. " << endl;
}