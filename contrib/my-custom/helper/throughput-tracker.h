#ifndef THROUGHPUT_TRACKER_H
#define THROUGHPUT_TRACKER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/nstime.h"

#include <queue>

namespace ns3
{

class ThroughputTracker : public Object
{
  public:
    static TypeId GetTypeId();

    ThroughputTracker();
    ThroughputTracker(uint32_t windowSize, DataRate initialRate = DataRate(uint64_t(1) << 20));
    ~ThroughputTracker() override;

    void DoInitialize() override;
    void DoDispose() override;

    void Update(uint32_t packetSize, Time now);
    DataRate GetThroughput() const;
    void Reset();

    uint64_t GetBitRate() const;

  private:
    uint32_t m_windowSize;
    std::queue<Time> m_timeRecord;
    std::queue<uint64_t> m_bufferRecord;
    uint64_t m_waitingTimeSum;
    uint64_t m_bufferSum;
    DataRate m_rate;
};

} // namespace ns3

#endif // THROUGHPUT_TRACKER_H
