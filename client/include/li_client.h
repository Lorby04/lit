#include "li.pb.h"
using namespace LI;
class LiClient{
public:
    LiClient(std::unique_ptr<brpc::Channel> aChannel)
    {
        mChannel = std::move(aChannel);
        mLogId = 0;
    }
public:    
    bool addTarget(Target &aTarget);
    bool queryTarget(Target &aTarget);
    bool eraseTarget(Target &aTarget);
    bool purge();

private:
    //std::unique_ptr<LiService_Stub> mStub;
    std::unique_ptr<brpc::Channel> mChannel;
    int mLogId;
};