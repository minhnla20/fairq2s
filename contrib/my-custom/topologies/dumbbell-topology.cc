#include "dumbbell-topology.h"

namespace ns3
{

DumbbellTopology::DumbbellTopology()
{
}

// Config socket function
static void
configSocket(uint32_t segmentSize, Ptr<Socket> oldSocket, Ptr<Socket> newSocket)
{
    if (newSocket != nullptr)
    {
        newSocket->SetAttribute("SegmentSize", UintegerValue(segmentSize));
    }
}

// Add tags function
static void
addTagsForSource(Ptr<const Packet> packet)
{
    // Add Send Time tag if not already present
    SendTimeTag sendTimeTag;
    sendTimeTag.SetAttribute("SendTime", TimeValue(Simulator::Now()));
    packet->AddByteTag(sendTimeTag);
}

static void
addEnqueueTimeTagForSource(Ptr<const Packet> packet)
{
    // Add Enqueue Time tag
    EnqueueTimeTag enqueueTimeTag;
    enqueueTimeTag.SetAttribute("EnqueueTime", TimeValue(Simulator::Now()));
    packet->AddByteTag(enqueueTimeTag);
}

static void
addTagsForACK(uint32_t applicationId, Ptr<const Packet> packet)
{
    // Add Application ID tag
    ApplicationIdTag applicationIdTag;
    applicationIdTag.SetAttribute("ApplicationId", UintegerValue(applicationId));
    packet->AddByteTag(applicationIdTag);

    // Add Send Time tag
    SendTimeTag sendTimeTag;
    sendTimeTag.SetAttribute("SendTime", TimeValue(Simulator::Now()));
    packet->AddByteTag(sendTimeTag);
}

void
DumbbellTopology::BuildTopology(const std::string& bottleneckBw)
{
    m_bottleneckBw = std::string(bottleneckBw);

    // Calculate after user inputs
    const uint32_t REALTIME_FLOWS_START_INDEX = 0;
    NS_LOG_UNCOND("REALTIME_FLOWS_START_INDEX: " << REALTIME_FLOWS_START_INDEX);
    const uint32_t REALTIME_FLOWS_END_INDEX = REALTIME_FLOWS_START_INDEX + NUM_REALTIME_FLOWS;
    NS_LOG_UNCOND("REALTIME_FLOWS_END_INDEX: " << REALTIME_FLOWS_END_INDEX);

    const uint32_t NON_REALTIME_FLOWS_START_INDEX = REALTIME_FLOWS_END_INDEX;
    NS_LOG_UNCOND("NON_REALTIME_FLOWS_START_INDEX: " << NON_REALTIME_FLOWS_START_INDEX);
    const uint32_t NON_REALTIME_FLOWS_END_INDEX =
        NON_REALTIME_FLOWS_START_INDEX + NUM_NON_REALTIME_FLOWS;
    NS_LOG_UNCOND("NON_REALTIME_FLOWS_END_INDEX: " << NON_REALTIME_FLOWS_END_INDEX);

    const uint32_t OTHER_FLOWS_START_INDEX = NON_REALTIME_FLOWS_END_INDEX;
    NS_LOG_UNCOND("OTHER_FLOWS_START_INDEX: " << OTHER_FLOWS_START_INDEX);
    const uint32_t OTHER_FLOWS_END_INDEX = OTHER_FLOWS_START_INDEX + NUM_OTHER_FLOWS;
    NS_LOG_UNCOND("OTHER_FLOWS_END_INDEX: " << OTHER_FLOWS_END_INDEX);

    // Packet header size: 54B = P2P (2B) + IPv4 (20B) + TCP (32B = 20B Standard + 12B Option)
    static uint32_t PACKET_HEADER_SIZE = 54;

    LEFT_LEAF_APP_PACKET_SIZES.resize(NUM_LEAFS);
    for (uint32_t i = REALTIME_FLOWS_START_INDEX; i < REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_PACKET_SIZES[i] = 64;
    }
    for (uint32_t i = NON_REALTIME_FLOWS_START_INDEX; i < NON_REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_PACKET_SIZES[i] = 1500;
    }
    for (uint32_t i = OTHER_FLOWS_START_INDEX; i < OTHER_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_PACKET_SIZES[i] = 256;
    }

    // Leaf bandwidths
    std::vector<std::string> LEAF_BWS(NUM_LEAFS);
    for (uint32_t i = 0; i < NUM_LEAFS; ++i)
    {
        LEAF_BWS[i] = "1Gbps";
    }

    // Leaf MTUs
    std::vector<uint32_t> LEAF_MTUS(NUM_LEAFS);
    for (uint32_t i = 0; i < NUM_LEAFS; ++i)
    {
        LEAF_MTUS[i] = BOTTLENECK_MTU;
    }

    LEFT_LEAF_DELAYS.resize(NUM_LEAFS);
    RIGHT_LEAF_DELAYS.resize(NUM_LEAFS);
    for (uint32_t i = REALTIME_FLOWS_START_INDEX; i < REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_DELAYS[i] = "1ms";
        RIGHT_LEAF_DELAYS[i] = "1us";
    }
    for (uint32_t i = NON_REALTIME_FLOWS_START_INDEX; i < NON_REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_DELAYS[i] = "10ms";
        RIGHT_LEAF_DELAYS[i] = "1us";
    }
    for (uint32_t i = OTHER_FLOWS_START_INDEX; i < OTHER_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_DELAYS[i] = "5ms";
        RIGHT_LEAF_DELAYS[i] = "1us";
    }

    static const std::string DEFAULT_QUEUE_TYPE = "ns3::DropTailQueue";
    static const std::string DEFAULT_QUEUE_SIZE = "10000p";

    // Application types
    LEFT_LEAF_APP_TYPES.clear();
    for (uint32_t i = REALTIME_FLOWS_START_INDEX; i < REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_TYPES.push_back(FlowType::REALTIME_FLOW);
    }
    for (uint32_t i = NON_REALTIME_FLOWS_START_INDEX; i < NON_REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_TYPES.push_back(FlowType::NON_REALTIME_FLOW);
    }
    for (uint32_t i = OTHER_FLOWS_START_INDEX; i < OTHER_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_TYPES.push_back(FlowType::OTHER_FLOW);
    }

    // Packet sending rate at the application layer.
    LEFT_LEAF_APP_BWS.clear();
    for (uint32_t i = REALTIME_FLOWS_START_INDEX; i < REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_BWS.push_back("1Mbps");
    }
    for (uint32_t i = NON_REALTIME_FLOWS_START_INDEX; i < NON_REALTIME_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_BWS.push_back("20Mbps");
    }
    for (uint32_t i = OTHER_FLOWS_START_INDEX; i < OTHER_FLOWS_END_INDEX; ++i)
    {
        LEFT_LEAF_APP_BWS.push_back("5Mbps");
    }

    // Schedule application start times
    // std::vector<Time> SCHEDULED_START_TIMES = {Seconds(0),
    //                                            Seconds(50),
    //                                            Seconds(100),
    //                                            Seconds(150),
    //                                            Seconds(200),
    //                                            Seconds(250),
    //                                            Seconds(300)};
    // std::vector<uint32_t> CONCURRENT_WORKING_REALTIME_FLOWS_COUNT{10, 20, 20, 60, 20, 20, 10};
    // std::vector<uint32_t> CONCURRENT_WORKING_NON_REALTIME_FLOWS_COUNT{10, 10, 30, 60, 30, 10,
    // 10}; std::vector<uint32_t> CONCURRENT_WORKING_OTHER_FLOWS_COUNT{10, 15, 25, 60, 25, 15, 10};

    uint32_t maxConcurrentWorkingRealTimeFlows =
        *std::max_element(CONCURRENT_WORKING_REALTIME_FLOWS_COUNT.begin(),
                          CONCURRENT_WORKING_REALTIME_FLOWS_COUNT.end());
    uint32_t maxConcurrentWorkingNonRealTimeFlows =
        *std::max_element(CONCURRENT_WORKING_NON_REALTIME_FLOWS_COUNT.begin(),
                          CONCURRENT_WORKING_NON_REALTIME_FLOWS_COUNT.end());
    uint32_t maxConcurrentWorkingOtherFlows =
        *std::max_element(CONCURRENT_WORKING_OTHER_FLOWS_COUNT.begin(),
                          CONCURRENT_WORKING_OTHER_FLOWS_COUNT.end());

    NS_ABORT_MSG_IF(maxConcurrentWorkingRealTimeFlows != NUM_REALTIME_FLOWS,
                    "The maximum number of concurrent working real-time flows must be equal to "
                    "NUM_REALTIME_FLOWS.");
    NS_ABORT_MSG_IF(maxConcurrentWorkingNonRealTimeFlows != NUM_NON_REALTIME_FLOWS,
                    "The maximum number of concurrent working non-real-time flows must be equal to "
                    "NUM_NON_REALTIME_FLOWS.");
    NS_ABORT_MSG_IF(
        maxConcurrentWorkingOtherFlows != NUM_OTHER_FLOWS,
        "The maximum number of concurrent working other flows must be equal to NUM_OTHER_FLOWS.");

    APPLICATION_SCHEDULED_TIMES.clear();
    for (uint32_t i = 0; i < SCHEDULED_START_TIMES.size(); ++i)
    {
        Time scheduledTime = SCHEDULED_START_TIMES[i];

        // Schedule Real-time flows
        uint32_t numActiveRealTimeFlows = CONCURRENT_WORKING_REALTIME_FLOWS_COUNT[i];
        for (uint32_t j = 0; j < NUM_REALTIME_FLOWS; ++j)
        {
            uint32_t applicationId = REALTIME_FLOWS_START_INDEX + j;
            if (j + 1 <= numActiveRealTimeFlows)
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].first == Seconds(-1))
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].first = scheduledTime;
                }
            }
            else
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].second <=
                    APPLICATION_SCHEDULED_TIMES[applicationId].first)
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].second = scheduledTime;
                }
            }
        }

        // Schedule Non-Real-time flows
        uint32_t numActiveNonRealTimeFlows = CONCURRENT_WORKING_NON_REALTIME_FLOWS_COUNT[i];
        for (uint32_t j = 0; j < NUM_NON_REALTIME_FLOWS; ++j)
        {
            uint32_t applicationId = NON_REALTIME_FLOWS_START_INDEX + j;
            if (j + 1 <= numActiveNonRealTimeFlows)
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].first == Seconds(-1))
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].first = scheduledTime;
                }
            }
            else
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].second <=
                    APPLICATION_SCHEDULED_TIMES[applicationId].first)
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].second = scheduledTime;
                }
            }
        }

        // Schedule Other flows
        uint32_t numActiveOtherFlows = CONCURRENT_WORKING_OTHER_FLOWS_COUNT[i];
        for (uint32_t j = 0; j < NUM_OTHER_FLOWS; ++j)
        {
            uint32_t applicationId = OTHER_FLOWS_START_INDEX + j;
            if (j + 1 <= numActiveOtherFlows)
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].first == Seconds(-1))
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].first = scheduledTime;
                }
            }
            else
            {
                if (APPLICATION_SCHEDULED_TIMES[applicationId].second <=
                    APPLICATION_SCHEDULED_TIMES[applicationId].first)
                {
                    APPLICATION_SCHEDULED_TIMES[applicationId].second = scheduledTime;
                }
            }
        }
    }

    // Ensure that all applications have a stop time
    for (uint32_t i = 0; i < NUM_LEAFS; ++i)
    {
        if (APPLICATION_SCHEDULED_TIMES[i].second == Seconds(-1))
        {
            APPLICATION_SCHEDULED_TIMES[i].second = Seconds(STOP_TIME);
        }
    }

    for (auto& [applicationId, scheduledTimes] : APPLICATION_SCHEDULED_TIMES)
    {
        NS_LOG_UNCOND("Application ID: "
                      << applicationId
                      << ", Scheduled Start Time: " << scheduledTimes.first.GetSeconds()
                      << ", Scheduled Stop Time: " << scheduledTimes.second.GetSeconds());
    }

    NS_LOG_UNCOND("===Creating nodes...===");
    m_routers.Create(NUM_ROUTERS);
    m_leftLeafs.Create(NUM_LEAFS);
    m_rightLeafs.Create(NUM_LEAFS);

    NS_LOG_UNCOND("===Creating links...===");
    PointToPointHelper p2pBottleneck;
    p2pBottleneck.SetDeviceAttribute("DataRate", StringValue(m_bottleneckBw));
    p2pBottleneck.SetDeviceAttribute("Mtu", UintegerValue(BOTTLENECK_MTU));
    p2pBottleneck.SetChannelAttribute("Delay", StringValue(BOTTLENECK_DELAY));

    // Do not queue after dequeue from QueueDisc, to avoid internal delay in the bottleneck
    // link. The queueing is already handled by the QueueDisc.
    p2pBottleneck.SetQueue(DEFAULT_QUEUE_TYPE, "MaxSize", StringValue("1p"));

    std::vector<PointToPointHelper> p2pLeftLeafs(NUM_LEAFS);
    std::vector<PointToPointHelper> p2pRightLeafs(NUM_LEAFS);

    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        p2pLeftLeafs[i].SetDeviceAttribute("DataRate", StringValue(LEAF_BWS[i]));
        p2pLeftLeafs[i].SetDeviceAttribute("Mtu", UintegerValue(LEAF_MTUS[i]));
        p2pLeftLeafs[i].SetChannelAttribute("Delay", StringValue(LEFT_LEAF_DELAYS[i]));
        p2pLeftLeafs[i].SetQueue(DEFAULT_QUEUE_TYPE, "MaxSize", StringValue(DEFAULT_QUEUE_SIZE));

        p2pRightLeafs[i].SetDeviceAttribute("DataRate", StringValue(LEAF_BWS[i]));
        p2pRightLeafs[i].SetDeviceAttribute("Mtu", UintegerValue(LEAF_MTUS[i]));
        p2pRightLeafs[i].SetChannelAttribute("Delay", StringValue(RIGHT_LEAF_DELAYS[i]));
        p2pRightLeafs[i].SetQueue(DEFAULT_QUEUE_TYPE, "MaxSize", StringValue(DEFAULT_QUEUE_SIZE));
    }

    NS_LOG_UNCOND("===Creating network devices...===");
    m_routersDevices = p2pBottleneck.Install(m_routers);

    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        NetDeviceContainer leftDevices =
            p2pLeftLeafs[i].Install(m_leftLeafs.Get(i), m_routers.Get(0));
        m_leftLeafDevices.Add(leftDevices.Get(0));
        m_leftRouterDevices.Add(leftDevices.Get(1));

        NetDeviceContainer rightDevices =
            p2pRightLeafs[i].Install(m_rightLeafs.Get(i), m_routers.Get(NUM_ROUTERS - 1));
        m_rightLeafDevices.Add(rightDevices.Get(0));
        m_rightRouterDevices.Add(rightDevices.Get(1));
    }

    NS_LOG_UNCOND("===Creating network stack...===");
    InternetStackHelper internet;
    internet.Install(m_routers);
    internet.Install(m_leftLeafs);
    internet.Install(m_rightLeafs);

    NS_LOG_UNCOND("===Config transport layer...===");
    // 2 MB (large enough) TCP buffers to prevent the applications from bottlenecking the exp
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(2 * 1e6));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(2 * 1e6));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(BOTTLENECK_MTU));
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(true));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
    Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));

    NS_LOG_UNCOND("===Create transport layer...===");
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        Ptr<TcpL4Protocol> tcpL4Left = m_leftLeafs.Get(i)->GetObject<TcpL4Protocol>();
        tcpL4Left->SetAttribute("SocketType", TypeIdValue(TypeId::LookupByName("ns3::TcpCubic")));

        Ptr<TcpL4Protocol> tcpL4Right = m_rightLeafs.Get(i)->GetObject<TcpL4Protocol>();
        tcpL4Right->SetAttribute("SocketType", TypeIdValue(TypeId::LookupByName("ns3::TcpCubic")));
    }

    NS_LOG_UNCOND("===Assigning IP addresses...===");
    Ipv4AddressHelper ipv4Left("10.0.0.0", "255.255.255.0");
    Ipv4AddressHelper ipv4Right("20.0.0.0", "255.255.255.0");
    Ipv4AddressHelper ipv4Router("100.0.0.0", "255.255.255.0");

    m_routerInterfaces = ipv4Router.Assign(m_routersDevices);

    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        Ipv4InterfaceContainer leftInterface = ipv4Left.Assign(
            NetDeviceContainer(m_leftLeafDevices.Get(i), m_leftRouterDevices.Get(i)));
        m_leftLeafInterfaces.Add(leftInterface.Get(0));
        m_leftRouterInterfaces.Add(leftInterface.Get(1));
        ipv4Left.NewNetwork();

        Ipv4InterfaceContainer rightInterface = ipv4Right.Assign(
            NetDeviceContainer(m_rightLeafDevices.Get(i), m_rightRouterDevices.Get(i)));
        m_rightLeafInterfaces.Add(rightInterface.Get(0));
        m_rightRouterInterfaces.Add(rightInterface.Get(1));
        ipv4Right.NewNetwork();
    }

    NS_LOG_UNCOND("===Populate routing table...===");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_UNCOND("===Create receiver applications...===");
    const uint32_t SINK_PORT = 9;
    m_serverApps = ApplicationContainer();
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                    InetSocketAddress(Ipv4Address::GetAny(), SINK_PORT));

        ApplicationContainer applications = sinkHelper.Install(m_rightLeafs.Get(i));

        applications.Start(APPLICATION_SCHEDULED_TIMES[i].first);
        applications.Stop(APPLICATION_SCHEDULED_TIMES[i].second);

        m_serverApps.Add(applications);
    }

    NS_LOG_UNCOND("===Create sender applications...===");
    m_clientApps = ApplicationContainer();
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        NS_LOG_UNCOND("Sender " << i << " -> Receiver " << i << " : "
                                << m_leftLeafInterfaces.GetAddress(i) << " -> "
                                << m_rightLeafInterfaces.GetAddress(i));

        ApplicationHelper applicationHelper("ns3::MyApplication");
        applicationHelper.SetAttribute("SourceId", UintegerValue(i));
        applicationHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
        applicationHelper.SetAttribute("FlowType", UintegerValue(LEFT_LEAF_APP_TYPES[i]));
        applicationHelper.SetAttribute("DataRate", DataRateValue(LEFT_LEAF_APP_BWS[i]));
        applicationHelper.SetAttribute(
            "PacketSize",
            UintegerValue(LEFT_LEAF_APP_PACKET_SIZES[i] + PACKET_HEADER_SIZE));
        applicationHelper.SetAttribute("OnTime",
                                       StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        applicationHelper.SetAttribute("OffTime",
                                       StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        applicationHelper.SetAttribute(
            "Remote",
            AddressValue(InetSocketAddress(m_rightLeafInterfaces.GetAddress(i), SINK_PORT)));

        ApplicationContainer applications = applicationHelper.Install(m_leftLeafs.Get(i));

        // Adjust the application to compensate the packet header size
        applications.Get(0)->GetObject<MyApplication>()->AdjustPacketSize(PACKET_HEADER_SIZE);
        applications.Get(0)->GetObject<MyApplication>()->AdjustSendingRate(PACKET_HEADER_SIZE);

        applications.Start(APPLICATION_SCHEDULED_TIMES[i].first);
        applications.Stop(APPLICATION_SCHEDULED_TIMES[i].second);

        m_clientApps.Add(applications);
    }

    // Configure socket segment size for each application
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        Ptr<MyApplication> clientApplication = m_clientApps.Get(i)->GetObject<MyApplication>();
        clientApplication->TraceConnectWithoutContext(
            "SocketTrace",
            MakeBoundCallback(&configSocket, LEFT_LEAF_APP_PACKET_SIZES[i]));
    }

    // Add tags before sending packet
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        // Add tag for source packet
        m_leftLeafDevices.Get(i)->TraceConnectWithoutContext("PhyTxBegin",
                                                             MakeCallback(&addTagsForSource));

        // Add tag for ACK packet (Sink packet)
        m_rightLeafDevices.Get(i)->TraceConnectWithoutContext("PhyTxBegin",
                                                              MakeBoundCallback(&addTagsForACK, i));
    }

    // Receive source packet and add enqueue time tag for source packet
    for (uint32_t i = 0; i < NUM_LEAFS; i++)
    {
        m_leftRouterDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxEnd",
            MakeCallback(&addEnqueueTimeTagForSource));
    }
}

} // namespace ns3
