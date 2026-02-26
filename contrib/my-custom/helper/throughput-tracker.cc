#include "throughput-tracker.h"

#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThroughputTracker");
NS_OBJECT_ENSURE_REGISTERED(ThroughputTracker);

TypeId
ThroughputTracker::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ThroughputTracker")
                            .SetParent<Object>()
                            .SetGroupName("TrafficControl")
                            .AddConstructor<ThroughputTracker>();
    return tid;
}

ThroughputTracker::ThroughputTracker()
    : m_windowSize(50),
      m_waitingTimeSum(0),
      m_bufferSum(0),
      m_rate(DataRate(uint64_t(1) << 20))
{
}

ThroughputTracker::ThroughputTracker(uint32_t windowSize, DataRate initialRate)
    : m_windowSize(windowSize),
      m_waitingTimeSum(0),
      m_bufferSum(0),
      m_rate(initialRate)
{
}

ThroughputTracker::~ThroughputTracker()
{
}

void
ThroughputTracker::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    // Initialize the tracker state here if needed
    Object::DoInitialize();
}

void
ThroughputTracker::DoDispose()
{
    NS_LOG_FUNCTION(this);
    // Clean up the tracker state here if needed
    Object::DoDispose();
}

void
ThroughputTracker::Update(uint32_t packetSize, Time now)
{
    if (m_timeRecord.empty())
    {
        m_timeRecord.push(now);
        m_bufferRecord.push(packetSize);
        return;
    }

    uint64_t waitingTime = now.GetNanoSeconds() - m_timeRecord.back().GetNanoSeconds();
    if (waitingTime > 0)
    {
        if (m_bufferRecord.size() < m_windowSize)
        {
            m_timeRecord.push(now);
            m_bufferRecord.push(packetSize);
            m_waitingTimeSum += waitingTime;
            m_bufferSum += packetSize;
        }
        else
        {
            Time frontTime = m_timeRecord.front();
            uint64_t frontBuffer = m_bufferRecord.front();

            m_timeRecord.pop();
            m_bufferRecord.pop();

            m_waitingTimeSum -= (m_timeRecord.front() - frontTime).GetNanoSeconds();
            m_bufferSum -= frontBuffer;

            m_timeRecord.push(now);
            m_bufferRecord.push(packetSize);
            m_waitingTimeSum += waitingTime;
            m_bufferSum += packetSize;
        }

        m_rate = DataRate(m_bufferSum * 8 * 1e9 / m_waitingTimeSum);
    }
}

void
ThroughputTracker::Reset()
{
    while (!m_timeRecord.empty())
    {
        m_timeRecord.pop();
        m_bufferRecord.pop();
    }
    m_waitingTimeSum = 0;
    m_bufferSum = 0;
    m_rate = DataRate(uint64_t(1) << 20); // Reset to initial rate
}

DataRate
ThroughputTracker::GetThroughput() const
{
    return m_rate;
}

uint64_t
ThroughputTracker::GetBitRate() const
{
    return m_rate.GetBitRate();
}
