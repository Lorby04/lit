#include <brpc/parallel_channel.h>
#include "li.pb.h"
using namespace LI;
class LiClient{
public:
    LiClient(std::shared_ptr<brpc::ParallelChannel> aChannel, size_t aIndex)
    {
        mChannel = aChannel;
        mLogId = int(aIndex) ;
        mIndex = aIndex;
    }
    size_t index(){return mIndex;}
public:    
    bool addTarget(Target &aTarget);
    bool queryTarget(Target &aTarget);
    bool eraseTarget(Target &aTarget);
    bool purge();

private:
    //std::unique_ptr<LiService_Stub> mStub;
    std::shared_ptr<brpc::ParallelChannel> mChannel;
    size_t mIndex;
    int mLogId;
};