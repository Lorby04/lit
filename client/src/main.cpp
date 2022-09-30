#include <gflags/gflags.h>
#include <bthread/bthread.h>
#include <butil/logging.h>
#include <butil/string_printf.h>
#include <butil/time.h>
#include <butil/macros.h>
#include <brpc/parallel_channel.h>
#include <brpc/server.h>
#include <string>
#include "li.pb.h"
#include "li_client.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

using namespace std;
using namespace LI;

DEFINE_int32(parallel_queries, 2, "Number of processes to query");
DEFINE_int32(parallel_adds, 2, "Number of threads to add");
DEFINE_uint64(target_num, 100, "Number of targets to be added");
DEFINE_uint64(target_queries, 1000, "Times to query targets");
DEFINE_int32(thread_num, 1, "Number of threads to send requests");
DEFINE_int32(channel_num, 1, "Number of sub channels");
DEFINE_bool(same_channel, false, "Add the same sub channel multiple times");
DEFINE_bool(use_bthread, true, "Use bthread to send requests");
DEFINE_int32(attachment_size, 0, "Carry so many byte attachment along with requests");
//DEFINE_int32(request_size, 16, "Bytes of each request");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(protocol, "http", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(server, "0.0.0.0:8000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)"); 
DEFINE_bool(dont_fail, false, "Print fatal when some call failed");
//DEFINE_int32(dummy_port, -1, "Launch dummy server at this port");
void generateTargets(LiClient &aClient){
    uint64_t range_start = aClient.index();//FLAGS_target_num*aClient.index()/aClient.threads();
    uint64_t range_end = aClient.index()+1;//FLAGS_target_num*(aClient.index()+1)/aClient.threads();
    unsigned long digit = 1000000001 + range_start;
    LOG(WARNING) << "Start to add from "<<range_start <<" to "<<range_end;
	//auto start = std::chrono::high_resolution_clock::now();
 	for (uint64_t i = range_start; i < range_end; i++ ){
		if (digit%10 == 0) {
			digit++;
		}
		Target t;
		t.set_key(to_string(digit));
		t.set_type_of_key(TypeOfTarget(i%TypeOfTarget_ARRAYSIZE));
		aClient.addTarget(t);
		digit++;
        //bthread_usleep(5000);
	}
	//auto end = std::chrono::high_resolution_clock::now();
    //auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	//LOG(WARNING) <<"Writing entries:" << added << " time: "  << elapsed <<"ns (Requested:" << range_end-range_start << ")" ;
}


void perfTest(LiClient &aClient){
    uint64_t range_start = aClient.index()/(2*TypeOfTarget_ARRAYSIZE);//FLAGS_target_queries*aClient.index()/aClient.threads();
    uint64_t range_end = aClient.index()/(2*TypeOfTarget_ARRAYSIZE)+1;//FLAGS_target_queries*(aClient.index()+1)/aClient.threads();

    int typeToQuey = int(aClient.index()%TypeOfTarget_ARRAYSIZE);
    bool isFromLow = (aClient.index()%(2*TypeOfTarget_ARRAYSIZE) < TypeOfTarget_ARRAYSIZE);

    unsigned long low  = 1000000001 + range_start;
    unsigned long high = 9999999999 - range_start;

    int n = 1;
	//auto start = std::chrono::high_resolution_clock::now();
    for (int round = 0; round < n; round++){
        for (uint64_t i = range_start; i<range_end;i++){
            if(isFromLow){
                if (low%10 == 0) {
                    low++;
                }

                //for (int ty=0;ty<TypeOfTarget_ARRAYSIZE;ty++) {
                {
                    int ty = typeToQuey;
                    Target t;
                    t.set_key(to_string(low));
                    t.set_type_of_key(TypeOfTarget(ty));
                    aClient.queryTarget(t);
                    bthread_usleep(50000);
                }
                low++;
            }else{
                if (high%10 == 0) {
                    high--;
                }

                //for (int ty=0;ty<TypeOfTarget_ARRAYSIZE;ty++) 
                {
                    int ty = typeToQuey;
                    Target t;
                    t.set_key(to_string(high));
                    t.set_type_of_key(TypeOfTarget(ty));
                    aClient.queryTarget(t);
                    bthread_usleep(50000);
                }
                high--;
            }
        }
    }
	//auto end = std::chrono::high_resolution_clock::now();
    //auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
	//cout << "Searching time:" << elapsed << "ns"; // ,Statistics: "<< statistics() << std::endl;
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


#if 0
DEFINE_string(connection_type, "pooled", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "0.0.0.0:8000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)"); 
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
DEFINE_string(protocol, "http", "Client-side protocol");
#endif
bvar::LatencyRecorder g_latency_recorder("client");
bvar::Adder<int> g_error_count("client_error_count");
bvar::LatencyRecorder* g_sub_channel_latency = NULL;

std::shared_ptr<brpc::ParallelChannel> makeChannel() {
    // A Channel represents a communication line to a Server. Notice that 
    // Channel is thread-safe and can be shared by all threads in your program.
    std::shared_ptr<brpc::ParallelChannel> channel = std::make_shared<brpc::ParallelChannel>();
    brpc::ParallelChannelOptions pchan_options;
    pchan_options.timeout_ms = FLAGS_timeout_ms;
    if (channel->Init(&pchan_options) != 0) {
        LOG(ERROR) << "Fail to init ParallelChannel";
        exit(-1);
    }

    brpc::ChannelOptions sub_options;
    sub_options.protocol = FLAGS_protocol;
    sub_options.connection_type = FLAGS_connection_type;
    sub_options.max_retry = FLAGS_max_retry;
    // Setting sub_options.timeout_ms does not work because timeout of sub 
    // channels are disabled in Parallelchannel->

    if (FLAGS_same_channel) {
        // For brpc >= 1.0.155.31351, a sub channel can be added into
        // a ParallelChannel more than once.
        brpc::Channel* sub_channel = new brpc::Channel;
        // Initialize the channel, NULL means using default options. 
        // options, see `brpc/channel->h'.
        if (sub_channel->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &sub_options) != 0) {
            LOG(ERROR) << "Fail to initialize sub_channel";
            exit(-1);
        }
        for (int i = 0; i < FLAGS_channel_num; ++i) {
            if (channel->AddChannel(sub_channel, brpc::OWNS_CHANNEL,
                                   NULL, NULL) != 0) {
                LOG(ERROR) << "Fail to AddChannel, i=" << i;
                exit(-1);
            }
        }
    } else {
        for (int i = 0; i < FLAGS_channel_num; ++i) {
            brpc::Channel* sub_channel = new brpc::Channel;
            // Initialize the channel, NULL means using default options. 
            // options, see `brpc/channel->h'.
            if (sub_channel->Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &sub_options) != 0) {
                LOG(ERROR) << "Fail to initialize sub_channel[" << i << "]";
                exit(-1);
            }
            if (channel->AddChannel(sub_channel, brpc::OWNS_CHANNEL,
                                   NULL, NULL) != 0) {
                LOG(ERROR) << "Fail to AddChannel, i=" << i;
                exit(-1);
            }
        }
    }
    return channel;

}

static void* creater(void* arg) {
    // Normally, you should not call a Channel directly, but instead construct
    // a stub Service wrapping it. stub can be shared by all threads as well.
    std::unique_ptr<LiClient> client(static_cast<LiClient *>(arg));
	LOG(WARNING)<<"Start to generate targets: " << client->index();
	generateTargets(*client);
    return nullptr;
}

static void* inquirer(void* arg) {
    // Normally, you should not call a Channel directly, but instead construct
    // a stub Service wrapping it. stub can be shared by all threads as well.
    std::unique_ptr<LiClient> client(static_cast<LiClient *>(arg));
	LOG(WARNING)<<"Start to inquiry targets: " << client->index();
    //sleep(100);
	perfTest(*client);
    return nullptr;
}

static uint64_t added = 0;
static uint64_t queried = 0;
static void childAdd(){
    uint64_t start = added;
    LOG(INFO)<<"Add No. "<<start;
    auto channel = makeChannel();
    // Initialize bvar for sub channel
    g_sub_channel_latency = new bvar::LatencyRecorder[FLAGS_channel_num];
    for (int i = 0; i < FLAGS_channel_num; ++i) {
        std::string name;
        butil::string_printf(&name, "client_sub_%d", i);
        g_sub_channel_latency[i].expose(name);
    }

/*
    if (FLAGS_attachment_size > 0) {
        g_attachment.resize(FLAGS_attachment_size, 'a');
    }
    if (FLAGS_request_size <= 0) {
        LOG(ERROR) << "Bad request_size=" << FLAGS_request_size;
        return -1;
    }
    g_request.resize(FLAGS_request_size, 'r');

    if (FLAGS_dummy_port >= 0) {
        brpc::StartDummyServerAt(FLAGS_dummy_port);
    }
*/

    std::vector<bthread_t> bids;
    std::vector<pthread_t> pids;
    if (!FLAGS_use_bthread) {
        pids.resize(FLAGS_thread_num);
        for (size_t i = 0; i < size_t(FLAGS_thread_num); ++i) {
            auto clientAdd = std::make_unique<LiClient>(channel, start);
            if (pthread_create(&pids[i], NULL, creater, clientAdd.get()) != 0) {
                LOG(ERROR) << "Fail to create pthread for add";
                exit(-1);
            }else{
                clientAdd.release();
            }
        }
    } else {
        bids.resize(FLAGS_thread_num);

        for (size_t i = 0; i < size_t(FLAGS_thread_num); ++i) {
            auto clientAdd = std::make_unique<LiClient>(channel, start);
            if (bthread_start_background(
                    &bids[i], NULL, creater, clientAdd.get()) != 0) {
                LOG(ERROR) << "Fail to create bthread for add";
                exit(-1);
            }else{
                clientAdd.release();
            }
        }
    }

    for (int i = 0; i < FLAGS_thread_num; ++i) {
        if (!FLAGS_use_bthread) {
            pthread_join(pids[i], NULL);
        } else {
            bthread_join(bids[i], NULL);
        }
    }
}

static void childQuery(){
    uint64_t start = queried;
    LOG(INFO)<<"Query No. "<<start;    
    auto channel = makeChannel();
    // Initialize bvar for sub channel
    g_sub_channel_latency = new bvar::LatencyRecorder[FLAGS_channel_num];
    for (int i = 0; i < FLAGS_channel_num; ++i) {
        std::string name;
        butil::string_printf(&name, "client_sub_%d", i);
        g_sub_channel_latency[i].expose(name);
    }

/*
    if (FLAGS_attachment_size > 0) {
        g_attachment.resize(FLAGS_attachment_size, 'a');
    }
    if (FLAGS_request_size <= 0) {
        LOG(ERROR) << "Bad request_size=" << FLAGS_request_size;
        return -1;
    }
    g_request.resize(FLAGS_request_size, 'r');

    if (FLAGS_dummy_port >= 0) {
        brpc::StartDummyServerAt(FLAGS_dummy_port);
    }
*/

    std::vector<bthread_t> bids;
    std::vector<pthread_t> pids;
    if (!FLAGS_use_bthread) {
        pids.resize(FLAGS_thread_num);
        for (size_t i = 0; i < size_t(FLAGS_thread_num); ++i) {
            auto clientAdd = std::make_unique<LiClient>(channel, start);
            if (pthread_create(&pids[i], NULL, inquirer, clientAdd.get()) != 0) {
                LOG(ERROR) << "Fail to create pthread for add";
                exit(-1);
            }else{
                clientAdd.release();
            }
        }
    } else {
        bids.resize(FLAGS_thread_num);

        for (size_t i = 0; i < size_t(FLAGS_thread_num); ++i) {
            auto clientAdd = std::make_unique<LiClient>(channel, start);
            if (bthread_start_background(
                    &bids[i], NULL, inquirer, clientAdd.get()) != 0) {
                LOG(ERROR) << "Fail to create bthread for add";
                exit(-1);
            }else{
                clientAdd.release();
            }
        }
    }

    for (int i = 0; i < FLAGS_thread_num; ++i) {
        if (!FLAGS_use_bthread) {
            pthread_join(pids[i], NULL);
        } else {
            bthread_join(bids[i], NULL);
        }
    }
}
static void* threadAdd(void*){
    pthread_atfork(nullptr,nullptr, childAdd);
    LOG(INFO) << "Start add from "<<added;
    
    for (; added<FLAGS_target_num; ++added){
        bthread_usleep(50000);
        std::vector<pid_t> cpids;
        for (int i = 0; i < FLAGS_parallel_adds; i++){
            int cpid = fork();
            if (cpid == 0){
                break;
            }else if (cpid<0){
                LOG(ERROR) << "Failed to fork process";
                break;
            }else{
                cpids.push_back(cpid);
                bthread_usleep(1000);
            }
        }

        for (size_t i = 0; i < cpids.size(); i++){
            if (cpids[i] != 0){
                int status;
                waitpid(cpids[i],&status, 0);
            }
        }
    }
    return nullptr;
}

static void* threadQuery(void*){
    pthread_atfork(nullptr,nullptr, childQuery);
    LOG(INFO) << "Start query from "<<queried;
//    sleep(100);

    for (; queried<FLAGS_target_num; ++queried){
        bthread_usleep(50000);
        std::vector<pid_t> cpids;

        for (int i = 0; i < FLAGS_parallel_queries; i++){
            int cpid = fork();
            if (cpid == 0){
                break;
            }else if (cpid<0){
                LOG(ERROR) << "Failed to fork process";
                break;
            }else{
                cpids.push_back(cpid);
                bthread_usleep(1000);
            }
        }

        for (size_t i = 0; i < cpids.size(); i++){
            if (cpids[i] != 0){
                int status;
                waitpid(cpids[i],&status, 0);
            }
        }
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    std::vector<pthread_t> pids;
    pids.resize(2);

    srand(time(NULL));
    int r = rand()%2;
    if (r == 0){
        if (pthread_create(&pids[0], NULL, threadAdd, nullptr) != 0) {
            LOG(ERROR) << "Fail to create pthread for add";
            return -1;
        }
    }else{
        if (pthread_create(&pids[1], NULL, threadQuery, nullptr) != 0) {
            LOG(ERROR) << "Fail to create pthread for add";
            return -1;
        }
    }
   while (!brpc::IsAskedToQuit()) {
        sleep(1);
        LOG(INFO) << "Sending EchoRequest at qps=" << g_latency_recorder.qps(1)
                  << " latency=" << g_latency_recorder.latency(1) << noflush;
        for (int i = 0; i < FLAGS_channel_num; ++i) {
            LOG(INFO) << " latency_" << i << "=" 
                      << g_sub_channel_latency[i].latency(1)
                      << noflush;
        }
        LOG(INFO);
    }

    LOG(INFO) << "EchoClient is going to quit";
    //for (int i = 0; i < 2; ++i) {
        pthread_join(pids[r], NULL);
    //}


    return 0;

	//perfTest(12, client);
}
