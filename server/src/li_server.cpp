#include "brpc/http_status_code.h"
#include "proto_addon.h"
#include "target_cache.h"
#include "li_server.h"

namespace LI{
    void ServerImpl::Query(google::protobuf::RpcController* controller,
                        const QueryRequest* request,
                        QueryResponse* response,
                        google::protobuf::Closure* done){
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should 
        // remove these logs in performance-sensitive servers.
        LOG(WARNING) << "Received request[log_id=" << cntl->log_id() 
                  << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": " << request->DebugString()
                  << ", URI: " << cntl->http_request().uri();

        // Fill response.
        bool found = TargetCache::found(request->target());
        if (!found){
            cntl->http_response().set_status_code(brpc::HTTP_STATUS_NOT_FOUND);
        }
        response->set_found(found);

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);  
    }

    void ServerImpl::Add(google::protobuf::RpcController* controller,
                        const AddRequest* request,
                        AddResponse* response,
                        google::protobuf::Closure* done){
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should 
        // remove these logs in performance-sensitive servers.
        LOG(WARNING) << "Received request[log_id=" << cntl->log_id() 
                  << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": " << request->DebugString()
                  << ", URI: " << cntl->http_request().uri();

        // Fill response.
        // Target in the request is moved out after the call
        bool success = TargetCache::insert(std::move(*(const_cast<AddRequest*>(request))->mutable_target()));
        if (!success){
            cntl->http_response().set_status_code(brpc::HTTP_STATUS_INTERNAL_SERVER_ERROR);
        }

        response->set_success(success);

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);  
    } 

    void ServerImpl::Del(google::protobuf::RpcController* controller,
                        const DelRequest* request,
                        DelResponse* response,
                        google::protobuf::Closure* done){
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(controller);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should 
        // remove these logs in performance-sensitive servers.
        LOG(WARNING) << "Received request[log_id=" << cntl->log_id() 
                  << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": " << request->DebugString()
                  << ", URI: " << cntl->http_request().uri();

        // Fill response.
        // Target in the request is moved out after the call
        bool success = TargetCache::erase(request->target());

        response->set_success(success);

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);  
    }    
}