#include "drop-tracker.h"

#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DropTracker");
NS_OBJECT_ENSURE_REGISTERED(DropTracker);

TypeId
DropTracker::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DropTracker")
                            .SetParent<Object>()
                            .SetGroupName("TrafficControl")
                            .AddConstructor<DropTracker>();
    return tid;
}

DropTracker::DropTracker()
    : m_maximumWaitingTime(Seconds(10)),
      m_dropSum(0)
{
}

DropTracker::DropTracker(Time maximumWaitingTime)
    : m_maximumWaitingTime(maximumWaitingTime),
      m_dropSum(0)
{
}

DropTracker::~DropTracker()
{
}

void
DropTracker::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    // Initialize the tracker state here if needed
    Object::DoInitialize();
}

void
DropTracker::DoDispose()
{
    NS_LOG_FUNCTION(this);
    // Clean up the tracker state here if needed
    Object::DoDispose();
}

void
DropTracker::Update(Time now, bool isDropped)
{
    while (m_timeRecord.size() > 0)
    {
        Time waitingTime = now - m_timeRecord.back();
        if (waitingTime <= m_maximumWaitingTime)
        {
            break;
        }

        m_timeRecord.pop();

        bool isDropped = m_dropRecord.front();
        m_dropRecord.pop();

        if (isDropped)
        {
            m_dropSum--;
        }
    }

    m_timeRecord.push(now);
    m_dropRecord.push(isDropped);
    if (isDropped)
    {
        m_dropSum++;
    }
}

double
DropTracker::GetDropRate()
{
    Update(Simulator::Now(), false);

    if (m_timeRecord.empty())
    {
        return 0.0;
    }

    uint32_t numPackets = m_timeRecord.size();

    if (numPackets + m_dropSum == 0)
    {
        return 0.0;
    }

    double dropRate = static_cast<double>(m_dropSum) / (numPackets + m_dropSum);
    return dropRate;
}

void
DropTracker::Reset()
{
    while (!m_timeRecord.empty())
    {
        m_timeRecord.pop();
        m_dropRecord.pop();
    }
    m_dropSum = 0;
}