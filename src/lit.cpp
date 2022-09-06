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

const string usage = "Usage: program-name ring|list -n decimal(total loops:1-10000) -s decimal(service routines)  -t [1:1000000]M|m|K|k map|hash\n";
const int typ   = 1;
const int np    = 2+1;
const int sp    = 4+1;
const int tp    = 6+1;
const int storage_index = 8;
const int number = 9;


int main(int argc, char **argv){
    if ((argc !=number) || (string(argv[np-1]) != "-n")){
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

	string chType = argv[typ];
	string storage = argv[storage_index];
	cout << "Testing started with parameters:channel type: " << chType<<", entries:"<< t*mk << ", query loops:" << n <<
		", service threads:" << s << ", storage:"<< storage<<std::endl;
	
	atomic_uint64_t test = 0;
	cout << "Atomic "<<(test.is_lock_free()?"IS":"is NOT") <<" lock free."<<std::endl;
	TargetSet::init(storage, t*mk);
    Service::init(chType, s)->generateTargets(t*mk);
	Service::getInstance()->perfTest(n);

	return 0;
}
