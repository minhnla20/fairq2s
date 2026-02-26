#include "application-trace.h"

namespace ns3
{

void
ApplicationTrace::Initialize()
{
    *qosStream->GetStream() << "Time(s),ApplicationId,FlowType,Throughput(Kbps),QoS(Kbps)"
                            << std::endl;
    *rttStream->GetStream() << "Time(s),ApplicationId,FlowType,ActualRTTForward(us),"
                               "ActualRTTBackward(us),ActualRTT(us),EstimatedRTT(us)"
                            << std::endl;
    *delayStream->GetStream() << "Time(s),ApplicationId,FlowType,DelayTime(us)" << std::endl;

    *sentReceivedBytesStream->GetStream()
        << "Time(s),ApplicationId,FlowType,SentBytes,ReceivedBytes" << std::endl;
    *retransmissionBytesStream->GetStream()
        << "Time(s),ApplicationId,FlowType,RetransmittedBytes" << std::endl;

    for (uint32_t i = 0; i < TOTAL_APPLICATIONS; ++i)
    {
        // Calculate expected QoS
        uint64_t applicationQoS = DataRate(APP_BWS[i]).GetBitRate();
        applicationId_QoS[i] = DataRate(applicationQoS);

        // Calculate expected RTT
        Time expectedRoundTimeTrip = Time(ApplicationTrace::LEFT_LEAF_DELAYS[i]) * 2;

        applicationId_RTT[i] = expectedRoundTimeTrip;
    }
}

void
ApplicationTrace::ApplicationSentReceivedBytesTrace(ApplicationTrace* applicationTrace,
                                               uint32_t applicationId,
                                               bool isSender,
                                               Ptr<const Packet> packet)
{
    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        if (isSender)
        {
            applicationTrace->applicationId_SentBytes[applicationId] += packet->GetSize();
        }
        else
        {
            applicationTrace->applicationId_ReceivedBytes[applicationId] += packet->GetSize();
        }
        return;
    }

    for (uint32_t i = 0; i < applicationTrace->TOTAL_APPLICATIONS; ++i)
    {
        *applicationTrace->sentReceivedBytesStream->GetStream()
            << currentTime.GetSeconds() << "," << i << ","
            << FLOW_TYPES[applicationTrace->APP_TYPES[i]] << ","
            << applicationTrace->applicationId_SentBytes[i] << ","
            << applicationTrace->applicationId_ReceivedBytes[i] << std::endl;

        applicationTrace->applicationId_SentBytes[i] = 0;
        applicationTrace->applicationId_ReceivedBytes[i] = 0;
    }

    if (isSender)
    {
        applicationTrace->applicationId_SentBytes[applicationId] += packet->GetSize();
    }
    else
    {
        applicationTrace->applicationId_ReceivedBytes[applicationId] += packet->GetSize();
    }

    lastTime = currentTime;
}

void
ApplicationTrace::RTTTrace(ApplicationTrace* applicationTrace,
                     uint32_t applicationId,
                     bool isForward,
                     Ptr<const Packet> packet)
{
    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    SendTimeTag sendTimeTag;
    bool hasTag = packet->FindFirstMatchingByteTag(sendTimeTag);
    if (!hasTag)
    {
        return;
    }

    Time sendTime = sendTimeTag.Get();
    Time rtt = currentTime - sendTime;

    if (elapsedTime < Time("200ms"))
    {
        if (isForward)
        {
            applicationTrace->applicationId_ActualRTTForwardRecords[applicationId].push_back(
                rtt);
            applicationTrace->applicationId_ActualRTTForwardTotal[applicationId] += rtt;
        }
        else
        {
            applicationTrace->applicationId_ActualRTTBackwardRecords[applicationId].push_back(
                rtt);
            applicationTrace->applicationId_ActualRTTBackwardTotal[applicationId] += rtt;
        }

        return;
    }

    for (uint32_t i = 0; i < applicationTrace->TOTAL_APPLICATIONS; ++i)
    {
        if (applicationTrace->applicationId_ActualRTTForwardRecords[i].empty() ||
            applicationTrace->applicationId_ActualRTTBackwardRecords[i].empty())
        {
            continue;
        }

        Time actualRTTForward =
            applicationTrace->applicationId_ActualRTTForwardTotal[i] /
            applicationTrace->applicationId_ActualRTTForwardRecords[i].size();
        Time actualRTTBackward =
            applicationTrace->applicationId_ActualRTTBackwardTotal[i] /
            applicationTrace->applicationId_ActualRTTBackwardRecords[i].size();

        Time actualRTT = actualRTTForward + actualRTTBackward;

        *applicationTrace->rttStream->GetStream()
            << currentTime.GetSeconds() << "," << i << ","
            << FLOW_TYPES[applicationTrace->APP_TYPES[i]] << ","
            << actualRTTForward.GetMicroSeconds() << "," << actualRTTBackward.GetMicroSeconds()
            << "," << actualRTT.GetMicroSeconds() << ","
            << applicationTrace->applicationId_RTT[i].GetMicroSeconds() << std::endl;

        applicationTrace->applicationId_ActualRTTForwardRecords[i].clear();
        applicationTrace->applicationId_ActualRTTForwardTotal[i] = Time(0);
        applicationTrace->applicationId_ActualRTTBackwardRecords[i].clear();
        applicationTrace->applicationId_ActualRTTBackwardTotal[i] = Time(0);
    }

    lastTime = currentTime;
}

void
ApplicationTrace::delayTrace(ApplicationTrace* applicationTrace, Ptr<const QueueDiscItem> item)
{
    static Time lastTime = Time(0);
    static default_map<uint32_t, Time> applicationId_MaxDelayTime{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    ApplicationIdTag applicationIdTag;
    item->GetPacket()->FindFirstMatchingByteTag(applicationIdTag);

    EnqueueTimeTag enqueueTimeTag;
    item->GetPacket()->FindFirstMatchingByteTag(enqueueTimeTag);

    if (elapsedTime < Time("200ms"))
    {
        Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
        applicationId_MaxDelayTime[applicationIdTag.Get()] =
            std::max(applicationId_MaxDelayTime[applicationIdTag.Get()], delayTime);

        return;
    }

    for (uint8_t i = 0; i < applicationTrace->TOTAL_APPLICATIONS; ++i)
    {
        uint32_t applicationId = i;
        uint32_t maxDelayTime = applicationId_MaxDelayTime[applicationId].GetMicroSeconds();

        *applicationTrace->delayStream->GetStream()
            << currentTime.GetSeconds() << "," << applicationId << ","
            << FLOW_TYPES[applicationTrace->APP_TYPES[applicationId]] << ","
            << maxDelayTime << std::endl;
    }
    applicationId_MaxDelayTime.clear();

    Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
    applicationId_MaxDelayTime[applicationIdTag.Get()] =
        std::max(applicationId_MaxDelayTime[applicationIdTag.Get()], delayTime);

    lastTime = currentTime;
}

void
ApplicationTrace::ApplicationRetransmissionBytesTrace(ApplicationTrace* applicationTrace,
                                                 uint32_t applicationId,
                                                 ns3::Ptr<const ns3::Packet> packet,
                                                 const ns3::TcpHeader& tcpHeader,
                                                 const ns3::Address& sourceAddress,
                                                 const ns3::Address& destinationAddress,
                                                 ns3::Ptr<const ns3::TcpSocketBase> tcpSocket)
{
    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        applicationTrace->applicationId_RetransmissionBytes[applicationId] += packet->GetSize();
        return;
    }

    for (uint32_t i = 0; i < applicationTrace->TOTAL_APPLICATIONS; ++i)
    {
        *applicationTrace->retransmissionBytesStream->GetStream()
            << currentTime.GetSeconds() << "," << i << ","
            << FLOW_TYPES[applicationTrace->APP_TYPES[i]] << ","
            << applicationTrace->applicationId_RetransmissionBytes[i] << std::endl;

        applicationTrace->applicationId_RetransmissionBytes[i] = 0;
    }

    applicationTrace->applicationId_RetransmissionBytes[applicationId] += packet->GetSize();

    lastTime = currentTime;
}

void
ApplicationTrace::SocketTrace(ApplicationTrace* applicationTrace,
                         uint32_t applicationId,
                         Ptr<Socket> oldSocket,
                         Ptr<Socket> newSocket)
{
    if (newSocket != nullptr)
    {
        newSocket->TraceConnectWithoutContext(
            "Retransmission",
            MakeBoundCallback(&ApplicationTrace::ApplicationRetransmissionBytesTrace,
                              applicationTrace,
                              applicationId));
    }
    else
    {
        oldSocket->TraceDisconnectWithoutContext(
            "Retransmission",
            MakeBoundCallback(&ApplicationTrace::ApplicationRetransmissionBytesTrace,
                              applicationTrace,
                              applicationId));
    }
}

void
ApplicationTrace::ApplicationThroughputTrace(ApplicationTrace* applicationTrace,
                                        uint32_t applicationId,
                                        Ptr<const Packet> packet)
{
    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    uint64_t packetSize = packet->GetSize() * 8; // In bits;

    if (elapsedTime < Time("200ms"))
    {
        applicationTrace->applicationId_ServerReceivedCumulativeBits[applicationId] +=
            packetSize;
        return;
    }

    for (uint32_t i = 0; i < applicationTrace->TOTAL_APPLICATIONS; ++i)
    {
        uint64_t applicationThroughput =
            applicationTrace->applicationId_ServerReceivedCumulativeBits[i] * 1e9 /
            elapsedTime.GetNanoSeconds();

        *applicationTrace->qosStream->GetStream()
            << currentTime.GetSeconds() << "," << i << ","
            << FLOW_TYPES[applicationTrace->APP_TYPES[i]] << ","
            << applicationThroughput / 1000 << ","
            << applicationTrace->applicationId_QoS[i].GetBitRate() / 1000 << std::endl;

        applicationTrace->applicationId_ServerReceivedCumulativeBits[i] = 0;
    }

    applicationTrace->applicationId_ServerReceivedCumulativeBits[applicationId] += packetSize;

    lastTime = currentTime;
}

} // namespace ns3
