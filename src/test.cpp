#include <iostream>
#include <thread>
#include <vector>
#include "li_lib.h"

////////////////////////////////
// test -n val
///////////////////////////////

#ifdef __cpp_lib_format
#define USAGE std::format("Usage: {} -n val[1:100]", argv[0])
#else
#define USAGE "Usage: "<<argv[0]<<" -n val[1:100]"
#endif

using namespace std;
int main(int argc, char **argv){
    if ((argc !=3) || (string(argv[1]) != "-n")){
        cout << USAGE << endl;
        exit(0);
    }

    int n = atoi(argv[2]);
    if (n < 1 || n > 100){
        cout << USAGE << endl;
        exit(0);
    }

    generateTargets();

    int thn = 4;

    {
        vector<jthread> thVector;
        for (int i = 0; i < thn; i++){
            thVector.push_back(jthread(perfTest, n, i));
        }
    }
    
    cout << "Main thread, all " <<thn << "threads are done"<< endl;
}

