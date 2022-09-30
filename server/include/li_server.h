#include "proto_addon.h"
#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>


namespace LI{
class ServerImpl : public LiService {
public:
    ServerImpl() {};
    virtual ~ServerImpl() {};
    virtual void Query(google::protobuf::RpcController* controller,
                        const QueryRequest* request,
                        QueryResponse* response,
                        google::protobuf::Closure* done);
    virtual void Add(google::protobuf::RpcController* controller,
                        const AddRequest* request,
                        AddResponse* response,
                        google::protobuf::Closure* done);
    virtual void Del(google::protobuf::RpcController* controller,
                        const DelRequest* request,
                        DelResponse* response,
                        google::protobuf::Closure* done);
    virtual void List(google::protobuf::RpcController* controller,
                        const ListRequest* request,
                        ListResponse* response,
                        google::protobuf::Closure* done){
        brpc::ClosureGuard done_guard(done);
    };
    virtual void Purge(google::protobuf::RpcController* controller,
                        const PurgeRequest* request,
                        PurgeResponse* response,
                        google::protobuf::Closure* done){
        brpc::ClosureGuard done_guard(done);                            
    };
};
}