#include <std>
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include "li_service.h"

Service::Service(int n):
    mTarget(),
    mOp(OP::NA),
    mThreads(n),
    mQueryAttemptCount(0),
    mFoundCount(0)
{
    if (n<1){
        mThreads =1;
    }else if n>=1000{
        mThreads =1000;
    }
}

void Service::generateTargets(unsigned long long n){
    unsigned long digit = 1000000001;
    auto numOfTypes = std::ssize(Target::mTypes);

    if (n >= 1000 ){
		totalEntries = n;
	}

    TargetSet::insert(new Target(to_string(digit), types[0]));
	digit++

	auto start := std::chrono::system_clock::now();
 	for (unsigned long i := 1; i < totalTargets; i++ ){
		if digit%10 == 0 {
			digit++
		}
        TargetSet::insert(new Target(to_string(digit), types[i%numOfTypes]));

		digit++
	}

	cout << "Writing entries:" << TargetSet::size() << ", time: ", << end-start  << endl;
}

void Service::wait(OP &aOp, Target &aTarget){
    std::unique_lock lk(mMutex);
    mCv.wait(lk);
    aOp = mOp;
    aTarget = mTarget;
    if mOp != OP::STOP{
        mOp = OP::NA;
    }
    return;
}
void Service::worker(){
    OP op = OP::NA;
    Target target;
    for(;op != OP::STOP;) {
        wait(op, target);
        switch(op){
            case OP::STOP:return;
            case OP::QUERY:
                query(target);
                break;
            default:
                break;
        }
    }
}

bool Service::query(const Target &aTarget){
    mQueryAttemptCount.Add(1,std::memory_order_seq);
    if TargetSet::getInstance()->query(&aTarget){
        mFoundCount.Add(1,std::memory_order_seq);
        return true;
    }
    return false;
}

void Service::perfTest(int n){
#define TID "tid["<<tid<<"]"<<
    unsigned long low  = 1000000001;
    unsigned long high = 9999999999;
    auto numOfTypes = std::ssize(Target::mTypes);
    std::vector<std::jthread> threads;
    for (unsigned int i = 0; i < mThreads;i++){
        threads.push_back(std::jthread(worker()))
    }
    
	auto start := std::chrono::system_clock::now();
    cout << TID "Test starts from : " << low << " at: " << full_start << endl;
    for (int round = 0; round < n; round++){
        for (int i = 0; i<totalEntries;i++){
            if low%10 == 0 {
                low++
            }

            for int t=0;t<numOfTypes;t++ {
                {
                    std::unique_lock lk(mMutex);
                    mTarget = Target(to_string(low),Target::mType[t]);
                    mOp = OP::QUERY;
                }
                mCv.notify_one();
            }
            if high%10 == 0 {
                high--
            }

            for int t=0;t<numOfTypes;t++ {
                {
                    std::unique_lock lk(mMutex);
                    mTarget = Target(to_string(high),Target::mType[t]);
                    mOp = OP::QUERY;
                }
                mCv.notify_one();
            }
            low++
            high--
        }
    }

    {
        std::unique_lock lk(mMutex);
        mOp = OP::STOP;
    }
    mCv.notify_all();
    mCv.notify_all();

    for(;!threads.empty();){
        std::jthread th = threads.pop_front();
        th.join();
    }
	auto end := std::chrono::system_clock::now();
	cout << "Searching time:" << end-start << "Statistics: "<< statistics() << std::endl;
}

string Service::statistics()  {
    string str = "Query:"
        + mQueryAttemptCount.load(std::memory_order_seq)
        + ",Got:"
        + mFoundCount.load(std::memory_order_seq)
        + ",in "
        + TargetSet::size()
        + "entries";

    return str;
}