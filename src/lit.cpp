#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include "li_target.h"
#include "li_service.h"

////////////////////////////////
// test -n val
///////////////////////////////

#ifdef __cpp_lib_format
#define USAGE usage//std::format("Usage: {} -n val[1:100]", argv[0])
#else
#define USAGE usage//"Usage: "<<argv[0]<<" -n val[1:100]"
#endif

using namespace std;

const string usage = "Usage: program-name -n decimal(total loops:1-10000) -s decimal(service routines)  -t [1:1000000]M|m|K|k\n";
const int np    = 2;
const int sp    = 4;
const int tp    = 6;


int main(int argc, char **argv){
    if ((argc !=7) || (string(argv[1]) != "-n")){
        cout << usage << endl;
        exit(0);
    }

    int n = atoi(argv[np]);
    if (n < 1 || n > 10000){
        cout << usage << endl;
        exit(0);
    }

	string ti = argv[tp];
    
	int mk = 1;
	if (ti.back() == 'M' || ti.back() == 'm') {
		mk = 1000000;
	} else if (ti.back() == 'K' || ti.back() == 'k') {
		mk = 1000;
	} else {
		mk = 1;
	}


    ti.pop_back();
	int t = stoi(ti);
	if (t < 1) {
		t = 1;
	}
	if (t > 100000) {
		t = 100000;
	}

    string si = argv[sp];
	int s = stoi(si);

	cout << "Testing started with parameters:entries:"<< t*mk << ", query loops:" << n <<
		", service threads:" << s << std::endl;

    Service::init(s)->generateTargets(t*mk);
	Service::getInstance()->perfTest(n);
	return 0;
}
