#include "rtqs-queue-disc.h"

#include "ns3/simulator.h"

#include <chrono>
#include <thread>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RtqsQueueDisc");
NS_OBJECT_ENSURE_REGISTERED(RtqsQueueDisc);

TypeId
RtqsQueueDisc::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RtqsQueueDisc")
            .SetParent<QueueDisc>()
            .SetGroupName("TrafficControl")
            .AddConstructor<RtqsQueueDisc>()
            .AddTraceSource("RtqsEnqueue",
                            "Enqueue a packet in the queue.",
                            MakeTraceSourceAccessor(&RtqsQueueDisc::m_traceRtqsEnqueue),
                            "ns3::RtqsQueueDisc::TracedCallback")
            .AddTraceSource("RtqsDequeue",
                            "Dequeue a packet from the queue.",
                            MakeTraceSourceAccessor(&RtqsQueueDisc::m_traceRtqsDequeue),
                            "ns3::RtqsQueueDisc::TracedCallback")
            .AddTraceSource("RtqsDrop",
                            "Drop a packet (for whatever reason).",
                            MakeTraceSourceAccessor(&RtqsQueueDisc::m_traceRtqsDrop),
                            "ns3::RtqsQueueDisc::TracedCallback");

    return tid;
}

RtqsQueueDisc::RtqsQueueDisc()
    : QueueDisc(QueueDiscSizePolicy::MULTIPLE_QUEUES)
{
    NS_LOG_FUNCTION(this);
}

RtqsQueueDisc::~RtqsQueueDisc()
{
    NS_LOG_FUNCTION(this);
}

void
RtqsQueueDisc::InitializeParams()
{
    NS_LOG_FUNCTION(this);
}

bool
RtqsQueueDisc::CheckConfig()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(GetNQueueDiscClasses() == 0,
                  "RtqsQueueDisc does not support queue disc classes");
    NS_ASSERT_MSG(GetNPacketFilters() == 0, "RtqsQueueDisc does not support packet filters");

    // The MaxSize set here is the maximum length of the physical device,
    // while the actual queue length is dynamic and obtained by getBuffer.
    if (GetNInternalQueues() == 0)
    {
        // Detection queue
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_DETECTION_QUEUE])));
        // 3 RealTime queues
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_REALTIME_QUEUE_1])));
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_REALTIME_QUEUE_2])));
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_REALTIME_QUEUE_3])));
        // non-realtime queue
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_NON_REALTIME_QUEUE])));
        // other queue
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_OTHER_QUEUE])));
        // Black Hole queue
        AddInternalQueue(CreateObjectWithAttributes<DropTailQueue<QueueDiscItem>>(
            "MaxSize",
            QueueSizeValue(max_queue_size[RtqsQueueId::RTQS_BLACK_HOLE_QUEUE])));
    }

    NS_ASSERT_MSG(GetNInternalQueues() == 7, "RtqsQueueDisc needs 7 internal queues");

    // The first interval to increase the queue length is the default time (10ms).

    Simulator::Schedule(DEFAULT_INCREASE_QUEUE_LEN_INTERVAL,
                        &RtqsQueueDisc::IncreaseQueueLength,
                        this,
                        RtqsQueueId::RTQS_REALTIME_QUEUE_1);
    Simulator::Schedule(DEFAULT_INCREASE_QUEUE_LEN_INTERVAL,
                        &RtqsQueueDisc::IncreaseQueueLength,
                        this,
                        RtqsQueueId::RTQS_REALTIME_QUEUE_2);
    Simulator::Schedule(DEFAULT_INCREASE_QUEUE_LEN_INTERVAL,
                        &RtqsQueueDisc::IncreaseQueueLength,
                        this,
                        RtqsQueueId::RTQS_REALTIME_QUEUE_3);

    return true;
}

void
RtqsQueueDisc::IncreaseQueueLength(RtqsQueueId queueId)
{
    QueueSize currentQueueSize = m_queueBufferSize[queueId];
    SetQueueBufferSize(queueId, currentQueueSize.GetValue() + 1);

    Time nextIncreaseTime =
        std::max(m_sendPacketDeltaT[queueId] * 10, DEFAULT_INCREASE_QUEUE_LEN_INTERVAL);
    Simulator::Schedule(nextIncreaseTime, &RtqsQueueDisc::IncreaseQueueLength, this, queueId);
}

RtqsQueueId
RtqsQueueDisc::DistributeToQueueId(uint32_t applicationId, FlowType flowType)
{
    NS_LOG_FUNCTION(this << applicationId << flowType);

    if (flowType == FlowType::HANDSHAKE_FLOW)
    {
        return RtqsQueueId::RTQS_DETECTION_QUEUE;
    }
    else if (flowType == FlowType::REALTIME_FLOW)
    {
        Time currentImportantTime = m_minADD[applicationId] + m_minADU[applicationId];
        if (currentImportantTime <=
            m_limitIndependentDelayRealtimeQueueId[RtqsQueueId::RTQS_REALTIME_QUEUE_1])
        {
            return RtqsQueueId::RTQS_REALTIME_QUEUE_1;
        }
        else if (currentImportantTime <=
                 m_limitIndependentDelayRealtimeQueueId[RtqsQueueId::RTQS_REALTIME_QUEUE_2])
        {
            return RtqsQueueId::RTQS_REALTIME_QUEUE_2;
        }
        else if (currentImportantTime <=
                 m_limitIndependentDelayRealtimeQueueId[RtqsQueueId::RTQS_REALTIME_QUEUE_3])
        {
            return RtqsQueueId::RTQS_REALTIME_QUEUE_3;
        }
        else
        {
            // The Realtime packet is assigned to Other Queue if the irrelevant delay is too large.
            return RtqsQueueId::RTQS_OTHER_QUEUE;
        }
    }
    else if (flowType == FlowType::NON_REALTIME_FLOW)
    {
        return RtqsQueueId::RTQS_NON_REALTIME_QUEUE;
    }
    else if (flowType == FlowType::OTHER_FLOW)
    {
        return RtqsQueueId::RTQS_OTHER_QUEUE;
    }
    // else if (type_id == 4)
    // {
    //     return 6;
    // }

    return RtqsQueueId::RTQS_OTHER_QUEUE;
}

void
RtqsQueueDisc::SetQueueBufferSize(RtqsQueueId queueId, uint32_t newSize)
{
    const uint32_t minSize = min_queue_size[queueId].GetValue();
    const uint32_t maxSize = max_queue_size[queueId].GetValue();

    uint32_t targetNewSize = std::min(maxSize, newSize);
    targetNewSize = std::max(minSize, targetNewSize);

    m_queueBufferSize[queueId] = QueueSize(QueueSizeUnit::PACKETS, targetNewSize);
}

Time
RtqsQueueDisc::GetCurrentQueueingTime(RtqsQueueId queueId)
{
    uint32_t queueingPacketSize = GetInternalQueue(queueId)->GetNPackets();
    Time currentDeltaT = m_sendPacketDeltaT[queueId];
    return queueingPacketSize * currentDeltaT;
}

Time
RtqsQueueDisc::GetMaxQueueingTime(RtqsQueueId queueId)
{
    uint32_t maxQueueingSize = m_queueBufferSize[queueId].GetValue();
    Time currentDeltaT = m_sendPacketDeltaT[queueId];
    return maxQueueingSize * currentDeltaT;
}

uint32_t
RtqsQueueDisc::GetEnqueueFlowCount(RtqsQueueId queueId)
{
    // Get current enqueue throughput recorder for Queue: queueId.
    default_map<uint32_t, uint32_t>& currentQueuePacketCount = m_queuePacketCount[queueId];

    // Count the number of flow in this queue.
    uint32_t flowCount = 0;
    for (auto it = currentQueuePacketCount.begin(); it != currentQueuePacketCount.end(); it++)
    {
        if (it->first != ApplicationIdTag::NOT_APPLICATION_ID && it->second > 0)
        {
            flowCount++;
        }
    }
    return flowCount;
}

void
RtqsQueueDisc::UpdateFairBandwidthShare()
{
    // NS_LOG_FUNCTION(this);

    default_map<RtqsQueueId, uint32_t> flowCountPerQueue{0};
    uint32_t flowCountAllQueues = 0;
    for (uint8_t queueId = RtqsQueueId::RTQS_REALTIME_QUEUE_1; queueId <= RtqsQueueId::RTQS_OTHER_QUEUE; queueId++)
    {
        uint32_t flowCount = GetEnqueueFlowCount(static_cast<RtqsQueueId>(queueId));
        flowCountPerQueue[static_cast<RtqsQueueId>(queueId)] = flowCount;
        flowCountAllQueues += flowCount;
    }

    if (flowCountAllQueues == 0)
    {
        for (uint8_t queueId = RtqsQueueId::RTQS_REALTIME_QUEUE_1; queueId <= RtqsQueueId::RTQS_OTHER_QUEUE;
             queueId++)
        {
            m_fairBandwidthShare[static_cast<RtqsQueueId>(queueId)] = 0;
        }
    }
    else
    {
        for (uint8_t queueId = RtqsQueueId::RTQS_REALTIME_QUEUE_1; queueId <= RtqsQueueId::RTQS_OTHER_QUEUE;
             queueId++)
        {
            m_fairBandwidthShare[static_cast<RtqsQueueId>(queueId)] =
                flowCountPerQueue[static_cast<RtqsQueueId>(queueId)] / double(flowCountAllQueues);
        }
    }
}

void
RtqsQueueDisc::UpdatePollingWeightParams(RtqsQueueId queueId, RtqsQueueId dequeuedRtqsQueueId)
{
    bool isDequeuedQueue = (dequeuedRtqsQueueId == queueId);

    if (isDequeuedQueue)
    {
        m_pollingDequeueValue[queueId] = 0;
    }
    else
    {
        m_pollingDequeueValue[queueId] += m_pollingDequeueFinal[dequeuedRtqsQueueId];
    }

    m_pollingDequeueFinal[queueId] =
        m_pollingDequeueWeight[queueId] - m_pollingDequeueValue[queueId];
}

void
RtqsQueueDisc::UpdateADDAndDownMPD(uint32_t applicationId, Time newADD, Time newDownMPD)
{
    // NS_LOG_FUNCTION(this << applicationId << newADD << newDownMPD);

    // Update ADD and minADD
    m_currentADD[applicationId] = newADD;
    if (!m_minADD.contains(applicationId))
    {
        m_minADD[applicationId] = newADD;
    }
    else if (newADD < m_minADD[applicationId])
    {
        m_minADD[applicationId] = newADD;
    }

    // Update DownMPD
    m_downMPD[applicationId] = newDownMPD;
}

uint32_t
RtqsQueueDisc::CalculatePoint(Ptr<Packet> packet, RtqsQueueId queueId)
{
    // cur_deltaT =0 means that this queue almost no queuing time at present
    // (the application packet sending rate is less than the available bandwidth).
    // So at that point, we do not need to adjust queue length.
    Time currentDeltaT = m_sendPacketDeltaT[queueId];
    if (currentDeltaT == Time(0))
    {
        return 0;
    }

    ApplicationIdTag applicationIdTag;
    packet->FindFirstMatchingByteTag(applicationIdTag);
    uint32_t applicationId = applicationIdTag.Get();

    Time currentMPD = std::max(m_upMPD[applicationId], m_downMPD[applicationId]);
    Time currentQueueingTime = GetCurrentQueueingTime(queueId);
    if (currentQueueingTime < currentMPD)
    {
        // Not the one with the largest delay currently.
        return 0;
    }

    // Calculate & Update Point value...
    Time currentADU = m_currentADU[applicationId];
    Time currentADD = m_currentADD[applicationId];
    Time currentMaxQueueingTime = GetMaxQueueingTime(queueId);
    if (currentADU + currentADD + currentMaxQueueingTime >= DELAY_INTERVAL_THRESHOLD)
    {
        return (currentADU + currentADD + currentMaxQueueingTime - DELAY_INTERVAL_THRESHOLD)
            .GetNanoSeconds();
    }

    return 0;
}

void
RtqsQueueDisc::UpdateFlowStatus(uint32_t applicationId, FlowStatus newFlowStatus)
{
    NS_LOG_FUNCTION(this << applicationId);

    m_flowStatus[applicationId] = newFlowStatus;
}

bool
RtqsQueueDisc::DoEnqueue(Ptr<QueueDiscItem> item)
{
    NS_LOG_FUNCTION(this << item);

    ApplicationIdTag applicationIdTag;
    bool hasApplicationIdTag = item->GetPacket()->FindFirstMatchingByteTag(applicationIdTag);

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    SendTimeTag sendTimeTag;
    item->GetPacket()->FindFirstMatchingByteTag(sendTimeTag);

    // ===Detect new flow===
    if (hasApplicationIdTag && m_flowStatus[applicationIdTag.Get()] == FlowStatus::NOT_STARTED)
    {
        m_flowStatus[applicationIdTag.Get()] = FlowStatus::STARTING;

        // Change the new flow to STABLE after a specific time
        Simulator::Schedule(Seconds(5),
                            &RtqsQueueDisc::UpdateFlowStatus,
                            this,
                            applicationIdTag.Get(),
                            FlowStatus::STABLE);
    }

    // If the min_ADD or min_ADU is empty -> This flow has just started to send packets.
    // If so, push all packets into the detection queue to obtain more accurate min_ADD and min_ADU.
    if (flowTypeTag.Get() == FlowType::REALTIME_FLOW)
    {
        if (!m_minADD.contains(applicationIdTag.Get()) ||
            !m_minADU.contains(applicationIdTag.Get()))
        {
            flowTypeTag.Set(FlowType::HANDSHAKE_FLOW);
        }
    }

    // Get Queue Id for do enqueue
    RtqsQueueId queueId = DistributeToQueueId(applicationIdTag.Get(), flowTypeTag.Get());

    // The realtime packet may also be assigned to Other Queue if the irrelevant delay is too large.
    // In this case, change the flow type to Other Flow (abandoned flow).
    if (queueId == RtqsQueueId::RTQS_OTHER_QUEUE)
    {
        flowTypeTag.Set(FlowType::OTHER_FLOW);
    }

    // TODO: ===Max Flow Backoff===
    // The flow without STABLE state shouldn't drop into Black Hole queue

    // ===Check and Resize Realtime Queues===
    bool isBelongToRealtimeQueue =
        queueId >= RtqsQueueId::RTQS_REALTIME_QUEUE_1 && queueId <= RtqsQueueId::RTQS_REALTIME_QUEUE_3;
    if (isBelongToRealtimeQueue)
    {
        // Calculate & Update Point value...
        uint32_t point = CalculatePoint(item->GetPacket(), queueId);

        if (point > 0)
        {
            // If the resize operation has been performed in a short period of time before,
            // and is still in the draining queue phase, no further adjustment is made.
            if (m_queueBufferSize[queueId] >= GetInternalQueue(queueId)->GetCurrentSize())
            {
                Time currentDeltaT = m_sendPacketDeltaT[queueId];
                // Calculate the resized queue length, that is, how long the queue length needs to
                // be reduced.
                uint32_t currentDrainPacketsNum = point / currentDeltaT.GetNanoSeconds();
                uint32_t newBufferSize =
                    m_queueBufferSize[queueId].GetValue() - currentDrainPacketsNum;

                NS_LOG_LOGIC("queueId: " << queueId);
                NS_LOG_LOGIC(
                    "currentADU: " << m_currentADU[applicationIdTag.Get()].GetMilliSeconds());
                NS_LOG_LOGIC(
                    "currentADD: " << m_currentADD[applicationIdTag.Get()].GetMilliSeconds());
                NS_LOG_LOGIC("GetMaxQueueingTime(queueId): "
                             << GetMaxQueueingTime(queueId).GetMilliSeconds());
                NS_LOG_LOGIC("currentQueueSize: " << GetInternalQueue(queueId)->GetCurrentSize());
                NS_LOG_LOGIC("point: " << point);
                NS_LOG_LOGIC("currentDeltaT: " << currentDeltaT.GetMilliSeconds());
                NS_LOG_LOGIC("currentDrainPacketsNum: " << currentDrainPacketsNum);
                NS_LOG_LOGIC("m_queueBufferSize[queueId].GetValue(): "
                             << m_queueBufferSize[queueId].GetValue());
                NS_LOG_LOGIC("newBufferSize: " << newBufferSize);
                // std::this_thread::sleep_for(std::chrono::milliseconds(100));

                SetQueueBufferSize(queueId, newBufferSize);
            }
        }
    }

    // ===Record UpMPD===
    MPDTag mpdTag;

    // Find the last matching ByteTag.
    // Because if there are multiple MPDs, they must be added in order, and the last one is the
    // latest MPD - This is the maximum per-hop delay
    ByteTagIterator byteTagIterator = item->GetPacket()->GetByteTagIterator();
    TypeId mpdTagId = mpdTag.GetInstanceTypeId();
    while (byteTagIterator.HasNext())
    {
        ByteTagIterator::Item byteTag = byteTagIterator.Next();
        if (byteTag.GetTypeId() == mpdTagId)
        {
            byteTag.GetTag(mpdTag);
        }
    }

    // Update UpMPD
    m_upMPD[applicationIdTag.Get()] = mpdTag.Get();

    // compare whether the current queueing time needs to be added to the packet as MPD.
    Time currentQueueingTime = GetCurrentQueueingTime(queueId);
    if (currentQueueingTime > m_upMPD[applicationIdTag.Get()])
    {
        m_upMPD[applicationIdTag.Get()] = currentQueueingTime;

        MPDTag newMpdTag;
        newMpdTag.SetAttribute("MPD", TimeValue(currentQueueingTime));
        item->GetPacket()->AddByteTag(newMpdTag);
    }

    // ===Record ADU===

    // Set the value of ADU
    Time currentADU = Simulator::Now() - sendTimeTag.Get();
    m_currentADU[applicationIdTag.Get()] = currentADU;

    // Update the value of min_ADU
    if (!m_minADU.contains(applicationIdTag.Get()))
    {
        m_minADU[applicationIdTag.Get()] = currentADU;
    }
    else if (currentADU < m_minADU[applicationIdTag.Get()])
    {
        m_minADU[applicationIdTag.Get()] = currentADU;
    }

    // ===Judge the packet that can enter the queue===
    if (GetInternalQueue(queueId)->GetNPackets() + 1 >
        m_queueBufferSize[queueId].GetValue())
    {
        // Let this queue will drain in Dequeue Phase
        if (isBelongToRealtimeQueue)
        {
            NS_LOG_LOGIC("Realtime Queue " << RTQS_QUEUE_IDS[queueId] << " is full, set to drain phase");
        }
        // For Black Hole queue, we will drop the first packet in this queue, and enter current one.
        else if (queueId == RtqsQueueId::RTQS_BLACK_HOLE_QUEUE)
        {
            Ptr<QueueDiscItem> frontItem = GetInternalQueue(RtqsQueueId::RTQS_BLACK_HOLE_QUEUE)->Dequeue();
            DropAfterDequeue(frontItem, "Queue is full, drop the first packet in Black Hole queue");
            m_traceRtqsDrop(this, RtqsQueueId::RTQS_BLACK_HOLE_QUEUE, frontItem);

            ApplicationIdTag frontItemApplicationIdTag;
            frontItem->GetPacket()->FindFirstMatchingByteTag(frontItemApplicationIdTag);
        }
        // For non Black Hole queue, we will drop this packet
        else
        {
            DropBeforeEnqueue(item, "Queue is full");
            NS_LOG_LOGIC("Drop packet before enqueue: Queue is full");
            m_traceRtqsDrop(this, queueId, item);

            return false;
        }
    }

    // ===Enqueue packet===
    bool isEnqueued = GetInternalQueue(queueId)->Enqueue(item);
    if (isEnqueued)
    {
        m_traceRtqsEnqueue(this, queueId, item);

        // Store 1 packet in queue packet count
        m_queuePacketCount[queueId][applicationIdTag.Get()]++;

        // Update queue weight
        m_pollingDequeueWeight[queueId] = 1 / double(GetEnqueueFlowCount(queueId));
        UpdateFairBandwidthShare();
    }
    else
    {
        m_traceRtqsDrop(this, queueId, item);
    }

    return isEnqueued;
}

void
RtqsQueueDisc::UpdateSendPacketDeltaT(RtqsQueueId queueId)
{
    NS_LOG_FUNCTION(this << queueId);

    // Get current send packet deltaT for queue: queue_id.
    Time waitingTime = Time(0);
    if (!m_sendPacketDeltaTLastRecord.contains(queueId))
    {
        m_sendPacketDeltaTLastRecord[queueId] = Simulator::Now();
    }

    // If the number of queueing packets for queue: queue_id is too small (<10), we believe that it
    // is not the bandwidth that limits the packet send interval, but the packet sending rate of the
    // current flow, so we believe that the packet sending interval can be 0.
    else if (GetInternalQueue(queueId)->GetNPackets() > 10)
    {
        waitingTime = Simulator::Now() - m_sendPacketDeltaTLastRecord[queueId];
    }
    // Update last record time
    m_sendPacketDeltaTLastRecord[queueId] = Simulator::Now();

    // // Initiate packet send deltaT queue for queue: queue_id.
    // if (!m_sendPacketDeltaTQueue.contains(queueId))
    // {
    //     m_sendPacketDeltaTQueue[queueId] = std::queue<Time>();
    //     m_sendPacketDeltaTSum[queueId] = Time(0);

    //     // At the begining, we assume that the packet_send_deltaT is zero, it means the queueing
    //     // time is zero
    //     m_sendPacketDeltaT[queueId] = Time(0);
    // }

    // Update the value of packet_send_deltaT for queue: queue_id
    if (m_sendPacketDeltaTQueue[queueId].size() < SEND_PACKET_DELTA_T_WINDOW)
    {
        m_sendPacketDeltaTQueue[queueId].push(waitingTime);
        m_sendPacketDeltaTSum[queueId] = m_sendPacketDeltaTSum[queueId] + waitingTime;
    }
    else
    {
        Time frontDeltaT = m_sendPacketDeltaTQueue[queueId].front();
        m_sendPacketDeltaTSum[queueId] = m_sendPacketDeltaTSum[queueId] - frontDeltaT;

        m_sendPacketDeltaTQueue[queueId].pop();
        m_sendPacketDeltaTQueue[queueId].push(waitingTime);
        m_sendPacketDeltaTSum[queueId] = m_sendPacketDeltaTSum[queueId] + waitingTime;
    }

    // std::queue<Time> tempQueue = m_sendPacketDeltaTQueue[queueId];
    // for (uint32_t i = 0; i < m_sendPacketDeltaTQueue[queueId].size(); i++)
    // {
    //     NS_LOG_LOGIC("m_sendPacketDeltaTQueue[" << RTQS_QUEUE_IDS[queueId] << "][" << i
    //                                             << "] = " <<
    //                                             tempQueue.front().GetMicroSeconds());
    //     tempQueue.pop();
    // }

    m_sendPacketDeltaT[queueId] =
        m_sendPacketDeltaTSum[queueId] / m_sendPacketDeltaTQueue[queueId].size();
}

Ptr<QueueDiscItem>
RtqsQueueDisc::DequeuePacket(RtqsQueueId queueId)
{
    NS_LOG_FUNCTION(this << queueId);

    Ptr<QueueDiscItem> item = GetInternalQueue(queueId)->Dequeue();
    NS_ASSERT(item != nullptr);

    m_traceRtqsDequeue(this, queueId, item);

    ApplicationIdTag applicationIdTag;
    item->GetPacket()->FindFirstMatchingByteTag(applicationIdTag);

    // Update the packets count for each queue.
    m_queuePacketCount[queueId][applicationIdTag.Get()]--;

    UpdateFairBandwidthShare();
    UpdateSendPacketDeltaT(queueId);

    NS_LOG_LOGIC("Dequeued item from queue " << RTQS_QUEUE_IDS[queueId]);
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return item;
}

Ptr<QueueDiscItem>
RtqsQueueDisc::DoDequeue()
{
    NS_LOG_FUNCTION(this);

    // The detection queue always takes precedence over the other queues.
    if (GetInternalQueue(RtqsQueueId::RTQS_DETECTION_QUEUE)->GetNPackets() > 0)
    {
        return DequeuePacket(RtqsQueueId::RTQS_DETECTION_QUEUE);
    }

    // If the current realtime queue is in the draining phase, the priority is higher than other
    // queues, but lower than the detection queue.
    for (uint32_t queueId = RtqsQueueId::RTQS_REALTIME_QUEUE_1; queueId <= RtqsQueueId::RTQS_REALTIME_QUEUE_3;
         queueId++)
    {
        if (GetInternalQueue(queueId)->GetNPackets() >
            m_queueBufferSize[static_cast<RtqsQueueId>(queueId)].GetValue())
        {
            return DequeuePacket(static_cast<RtqsQueueId>(queueId));
        }
    }

    // Polling-based dequeue strategy
    std::deque<uint8_t> minPollingDequeueFinal_RtqsQueueIds;
    for (uint8_t i = RtqsQueueId::RTQS_REALTIME_QUEUE_1; i <= RtqsQueueId::RTQS_OTHER_QUEUE; i++)
    {
        minPollingDequeueFinal_RtqsQueueIds.push_back(i);
    }

    std::sort(minPollingDequeueFinal_RtqsQueueIds.begin(),
              minPollingDequeueFinal_RtqsQueueIds.end(),
              [&](uint8_t queueIdA, uint8_t queueIdB) {
                  return m_pollingDequeueFinal[static_cast<RtqsQueueId>(queueIdA)] <
                         m_pollingDequeueFinal[static_cast<RtqsQueueId>(queueIdB)];
              });

    Ptr<QueueDiscItem> dequeuedItem = nullptr;
    RtqsQueueId dequeuedRtqsQueueId;
    bool foundDequeuedRtqsQueueId = false;
    for (uint8_t minPollingDequeueFinal_RtqsQueueId : minPollingDequeueFinal_RtqsQueueIds)
    {
        RtqsQueueId minRtqsQueueId = static_cast<RtqsQueueId>(minPollingDequeueFinal_RtqsQueueId);

        if (GetInternalQueue(minRtqsQueueId)->GetNPackets() == 0)
        {
            NS_LOG_LOGIC("Queue " << RTQS_QUEUE_IDS[minRtqsQueueId] << " is empty");
            m_pollingDequeueValue[minRtqsQueueId] = m_pollingDequeueWeight[minRtqsQueueId];
            continue;
        }

        if (!foundDequeuedRtqsQueueId)
        {
            dequeuedItem = DequeuePacket(static_cast<RtqsQueueId>(minRtqsQueueId));
            NS_LOG_LOGIC("Dequeued item from queue " << RTQS_QUEUE_IDS[minRtqsQueueId]);

            UpdatePollingWeightParams(minRtqsQueueId, minRtqsQueueId);

            dequeuedRtqsQueueId = minRtqsQueueId;
            foundDequeuedRtqsQueueId = true;
            continue;
        }

        NS_LOG_LOGIC("Update polling weight parameters for queue " << RTQS_QUEUE_IDS[minRtqsQueueId]);
        UpdatePollingWeightParams(minRtqsQueueId, dequeuedRtqsQueueId);
    }

    // for (uint8_t minPollingDequeueFinal_RtqsQueueId : minPollingDequeueFinal_RtqsQueueIds)
    // {
    //     RtqsQueueId minRtqsQueueId = static_cast<RtqsQueueId>(minPollingDequeueFinal_RtqsQueueId);
    //     NS_LOG_LOGIC("Queue " << RTQS_QUEUE_IDS[minRtqsQueueId]
    //                           << " polling weight: " << m_pollingDequeueWeight[minRtqsQueueId]
    //                           << " polling value: " << m_pollingDequeueValue[minRtqsQueueId]
    //                           << " polling final: " << m_pollingDequeueFinal[minRtqsQueueId]);
    // }

    if (!dequeuedItem)
    {
        NS_LOG_LOGIC("No item was dequeued");
    }

    return dequeuedItem;
}
