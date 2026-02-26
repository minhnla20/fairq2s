#ifndef DROP_TRACKER_H
#define DROP_TRACKER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/nstime.h"

#include <queue>

namespace ns3
{

class DropTracker : public Object
{
  public:
    static TypeId GetTypeId();

    DropTracker();
    DropTracker(Time maximumWaitingTime);
    ~DropTracker() override;

    void DoInitialize() override;
    void DoDispose() override;
    
    void Update(Time now, bool isDropped = true);
    double GetDropRate();
    void Reset();

  private:
    std::queue<Time> m_timeRecord;
    std::queue<bool> m_dropRecord;
    Time m_maximumWaitingTime;
    uint64_t m_dropSum;
};

} // namespace ns3

#endif // DROP_TRACKER_H
