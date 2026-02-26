#ifndef APPLICATION_TRACE_H
#define APPLICATION_TRACE_H

#include "../helper/my-mapping.h"
#include "../tags/flow-type-tag.h"
#include "../tags/send-time-tag.h"
#include "../tags/enqueue-time-tag.h"
#include "../tags/application-id-tag.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <vector>
#include <tuple>
#include <string>

namespace ns3
{

class ApplicationTrace
{
  public:
    Ptr<OutputStreamWrapper> qosStream;
    Ptr<OutputStreamWrapper> rttStream;
    Ptr<OutputStreamWrapper> delayStream;

    Ptr<OutputStreamWrapper> sentReceivedBytesStream;
    Ptr<OutputStreamWrapper> retransmissionBytesStream;

    std::vector<std::string> APP_BWS;
    std::vector<FlowType> APP_TYPES;
    std::vector<std::string> LEFT_LEAF_DELAYS;
    std::string BOTTLENECK_DELAY;
    uint32_t TOTAL_APPLICATIONS;

    Time SAMPLE_INTERVAL = Time("10s");
    Time LOG_INTERVAL = Time("200ms");
    Time lastTime{0};

    default_map<uint32_t, DataRate> applicationId_QoS;
    default_map<uint32_t, Time> applicationId_RTT;

    default_map<uint32_t, uint64_t> applicationId_SentBytes{0};
    default_map<uint32_t, uint64_t> applicationId_ReceivedBytes{0};

    default_map<uint32_t, std::vector<Time>> applicationId_ActualRTTForwardRecords{};
    default_map<uint32_t, Time> applicationId_ActualRTTForwardTotal{};
    default_map<uint32_t, std::vector<Time>> applicationId_ActualRTTBackwardRecords{};
    default_map<uint32_t, Time> applicationId_ActualRTTBackwardTotal{};

    default_map<uint32_t, uint64_t> applicationId_RetransmissionBytes{0};
    default_map<uint32_t, uint64_t> applicationId_ServerReceivedCumulativeBits{0};

    void Initialize();

    static void ApplicationSentReceivedBytesTrace(ApplicationTrace* applicationTrace,
                                                   uint32_t applicationId,
                                                   bool isSender,
                                                   Ptr<const Packet> packet);

    static void RTTTrace(ApplicationTrace* applicationTrace,
                         uint32_t applicationId,
                         bool isForward,
                         Ptr<const Packet> packet);

    static void delayTrace(ApplicationTrace* applicationTrace, Ptr<const QueueDiscItem> item);

    static void ApplicationRetransmissionBytesTrace(ApplicationTrace* applicationTrace,
                                                     uint32_t applicationId,
                                                     ns3::Ptr<const ns3::Packet> packet,
                                                     const ns3::TcpHeader& tcpHeader,
                                                     const ns3::Address& sourceAddress,
                                                     const ns3::Address& destinationAddress,
                                                     ns3::Ptr<const ns3::TcpSocketBase> tcpSocket);

    static void SocketTrace(ApplicationTrace* applicationTrace,
                             uint32_t applicationId,
                             Ptr<Socket> oldSocket,
                             Ptr<Socket> newSocket);

    static void ApplicationThroughputTrace(ApplicationTrace* applicationTrace,
                                            uint32_t applicationId,
                                            Ptr<const Packet> packet);
};

} // namespace ns3

#endif // APPLICATION_TRACE_H