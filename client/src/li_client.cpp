#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/channel.h>
#include <string>
#include "li.pb.h"
#include "li_client.h"

using namespace std;
using namespace LI;
bool LiClient::addTarget(Target &aTarget) {
    AddRequest request;
    AddResponse response;
    brpc::Controller cntl;
    string url = "localhost/Add";

    request.set_allocated_target(&aTarget);
    cntl.set_log_id(mLogId ++);  // set by user
    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
    cntl.http_request().uri() = url;
    LOG(WARNING) << "Before add cntl";

    LiService_Stub stub(mChannel.get());
    stub.Add(&cntl, &request, &response, NULL);
    LOG(WARNING) << "After add cntl";
    if (!cntl.Failed()) {
        LOG(INFO) << "Received response from " << cntl.remote_side()
            << ", to " << cntl.local_side()
            << ", Success: " << response.success() 
            << ", latency=" << cntl.latency_us() << "us";
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    return 0;
}

bool LiClient::queryTarget(Target &aTarget) {
    QueryRequest request;
    QueryResponse response;
    brpc::Controller cntl;
    string url = "localhost/Query";

    request.set_allocated_target(&aTarget);
    cntl.set_log_id(mLogId ++);  // set by user
    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    cntl.http_request().set_method(brpc::HTTP_METHOD_GET);
    cntl.http_request().uri() = url;
    LiService_Stub stub(mChannel.get());
    stub.Query(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        LOG(INFO) << "Received response from " << cntl.remote_side()
            << ", to " << cntl.local_side()
            << ", Found: " << response.found() 
            << ", latency=" << cntl.latency_us() << "us";
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    return 0;
}

bool LiClient::eraseTarget(Target &aTarget) {
    DelRequest request;
    DelResponse response;
    brpc::Controller cntl;
    string url = "localhost/Del";

    request.set_allocated_target(&aTarget);
    cntl.set_log_id(mLogId ++);  // set by user
    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    cntl.http_request().set_method(brpc::HTTP_METHOD_DELETE);
    cntl.http_request().uri() = url;
    LiService_Stub stub(mChannel.get());
    stub.Del(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        LOG(INFO) << "Received response from " << cntl.remote_side()
            << ", to " << cntl.local_side()
            << ", Success: " << response.success() 
            << ", latency=" << cntl.latency_us() << "us";
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    return 0;
}

bool LiClient::purge() {
    PurgeRequest request;
    PurgeResponse response;
    brpc::Controller cntl;
    string url = "localhost/Purge";

    cntl.set_log_id(mLogId ++);  // set by user
    // Because `done'(last parameter) is NULL, this function waits until
    // the response comes back or error occurs(including timedout).
    cntl.http_request().set_method(brpc::HTTP_METHOD_DELETE);
    cntl.http_request().uri() = url;
    LiService_Stub stub(mChannel.get());
    stub.Purge(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        LOG(INFO) << "Received response from " << cntl.remote_side()
            << " to " << cntl.local_side()
            << " latency=" << cntl.latency_us() << "us";
    } else {
        LOG(WARNING) << cntl.ErrorText();
    }
    return 0;
}