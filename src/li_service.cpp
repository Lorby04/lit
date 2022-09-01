
#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include "li_target.h"
#include "li_service.h"

Service* Service::mInstance = nullptr;
std::once_flag Service::mfInst;

Service::Service(int n):
    mThreads(n),
    mQueryAttemptCount(0),
    mFoundCount(0),
    mThreadEnd(0)
{
    if (n<1){
        mThreads =1;
    }else if (n>=1000){
        mThreads =1000;
    }
    mChannel = Channel<Message>::create(1000);
    mTotalEntries = 10000000;
}

void Service::generateTargets(uint64_t n){
    unsigned long digit = 1000000001;
    auto numOfTypes = TargetSet::types().size();

    if (n >= 1000 ){
		mTotalEntries = n;
	}

    TargetSet::insert(Target(to_string(digit), TargetSet::types()[0]));
	digit++;

	auto start = std::chrono::high_resolution_clock::now();
 	for (uint64_t i = 1; i < mTotalEntries; i++ ){
		if (digit%10 == 0) {
			digit++;
		}
        TargetSet::insert(Target(to_string(digit), TargetSet::types()[i%numOfTypes]));

		digit++;
	}
	auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	cout <<"Writing entries:" << TargetSet::size() << " time: "  << elapsed <<"ns (Requested:" << mTotalEntries << ")" << endl;
}

void Service::worker(int aId){
    try{
        cout << "Thread " << aId << ": started" << endl;
        int received = 0;
        for(;;) {
            auto m = mChannel->receive();
            ++received;
            switch(m->mOp){
                case OP::STOP:return;
                case OP::QUERY:
                    query(*m->mTarget);
                    break;
                default:
                    break;
            }
        }
    }catch(CloseException e){
        cout << "Channel is closed. " << std::endl;
    }catch(ShutdownException e){
        cout << "Channel is shutdown. " << std::endl;
    }
    mThreadEnd.release();
    return;
}

bool Service::query(Target &aTarget){
    mQueryAttemptCount++;
    if (TargetSet::found(aTarget)){
        mFoundCount++;
        return true;
    }
    return false;
}

void Service::perfTest(int n){
    unsigned long low  = 1000000001;
    unsigned long high = 9999999999;
    auto numOfTypes = TargetSet::types().size();
    //std::vector<std::jthread> threads;
    for (unsigned int i = 0; i < mThreads;i++){
        //threads.push_back(std::jthread(Service::thread_entry,this,i));
        std::jthread(Service::thread_entry,this,i).detach();
    }
    
	auto start = std::chrono::high_resolution_clock::now();
    for (int round = 0; round < n; round++){
        for (uint64_t i = 0; i<mTotalEntries;i++){
            if (low%10 == 0) {
                low++;
            }

//            cout << "Query Round:"<<round<<", ith:"<<i<<std::endl;
            for (int t=0;t<numOfTypes;t++) {
                std::unique_ptr<Target> target(new Target(to_string(low),TargetSet::types()[t]));
                std::unique_ptr<Message> m(new Message(std::move(target), OP::QUERY));
                mChannel->send(std::move(m));
            }

            if (high%10 == 0) {
                high--;
            }

            for (int t=0;t<numOfTypes;t++) {
                std::unique_ptr<Target> target(new Target(to_string(high),TargetSet::types()[t]));
                std::unique_ptr<Message> m(new Message(std::move(target), OP::QUERY));
                mChannel->send(std::move(m));
            }
            low++;
            high--;
        }
    }
    mChannel->close();
/*    
    for(;!threads.empty();){
        std::jthread &th = threads.back();
        th.join();
        threads.pop_back();
    }
*/
    mThreadEnd.acquire();// On close, one is ended means all are ended
	auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	cout << "Searching time:" << elapsed << "ns ,Statistics: "<< statistics() << std::endl;
}

string Service::statistics()  {
    string str = "Query:"
        + to_string(mQueryAttemptCount.load(std::memory_order_seq_cst))
        + ", Got:"
        + to_string(mFoundCount.load(std::memory_order_seq_cst))
        + ", in "
        + to_string(TargetSet::size())
        + " entries";

    return str;
}