#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/channel.h>
#include <string>
#include "li.pb.h"
#include "li_client.h"

using namespace std;
using namespace LI;

void generateTargets(uint64_t n, LiClient &aClient){
    unsigned long digit = 1000000001;
	uint64_t added = 0;

	auto start = std::chrono::high_resolution_clock::now();
 	for (uint64_t i = 1; i < n; i++ ){
		if (digit%10 == 0) {
			digit++;
		}
		Target t;
		t.set_key(to_string(digit));
		t.set_type_of_key(TypeOfTarget(i%TypeOfTarget_ARRAYSIZE));
		if (aClient.addTarget(t)){
			added++;
		}
		digit++;
	}
	auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	LOG(WARNING) <<"Writing entries:" << added << " time: "  << elapsed <<"ns (Requested:" << n << ")" ;
}


void perfTest(int n, LiClient &aClient){
    unsigned long low  = 1000000001;
    unsigned long high = 9999999999;
	uint64_t totalNum = 2000000;
	auto start = std::chrono::high_resolution_clock::now();
    for (int round = 0; round < n; round++){
        for (uint64_t i = 0; i<totalNum;i++){
            if (low%10 == 0) {
                low++;
            }

            for (int ty=0;ty<TypeOfTarget_ARRAYSIZE;ty++) {
                Target t;
				t.set_key(to_string(low));
				t.set_type_of_key(TypeOfTarget(ty));
				aClient.queryTarget(t);
            }

            if (high%10 == 0) {
                high--;
            }

            for (int ty=0;ty<TypeOfTarget_ARRAYSIZE;ty++) {
                Target t;
				t.set_key(to_string(high));
				t.set_type_of_key(TypeOfTarget(ty));
				aClient.queryTarget(t);
            }
            low++;
            high--;
        }
    }
	auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	cout << "Searching time:" << elapsed << "ns"; // ,Statistics: "<< statistics() << std::endl;
}

#if 0

bool Service::query(Target &aTarget){
    mQueryAttemptCount++;
    if (TargetSet::found(aTarget)){
        mFoundCount++;
        return true;
    }
    return false;
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

#endif

DEFINE_string(connection_type, "pooled", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:8000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)"); 
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
DEFINE_string(protocol, "http", "Client-side protocol");

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);
    
    // A Channel represents a communication line to a Server. Notice that 
    // Channel is thread-safe and can be shared by all threads in your program.
    std::unique_ptr<brpc::Channel> channel = std::make_unique<brpc::Channel>();
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }
	auto client = LiClient(std::move(channel));
	LOG(WARNING)<<"Start to generate targets";
	generateTargets(1000000, client);
	perfTest(12, client);
}
