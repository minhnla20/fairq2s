#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/my-custom-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <chrono>
#include <filesystem>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FqCoDelSimulation");

// ===Global variables===

// ===Tracing functions===
static void
enqueueFlowTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint64_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    if (elapsedTime < Time("200ms"))
    {
        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativePackets = flowType_CumulativePackets[flowType];
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << cumulativePackets << std::endl;
        flowType_CumulativePackets[flowType] = 0;
    }

    flowType_CumulativePackets.clear();

    flowType_CumulativePackets[flowTypeTag.Get()] += 1;
    lastTime = currentTime;
}

static void
dequeueFlowTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint64_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    if (elapsedTime < Time("200ms"))
    {
        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativePackets = flowType_CumulativePackets[flowType];
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << cumulativePackets << std::endl;
        flowType_CumulativePackets[flowType] = 0;
    }

    flowType_CumulativePackets.clear();

    flowType_CumulativePackets[flowTypeTag.Get()] += 1;
    lastTime = currentTime;
}

static void
dropFlowTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint64_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    if (elapsedTime < Time("200ms"))
    {
        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativePackets = flowType_CumulativePackets[flowType];
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << cumulativePackets << std::endl;
        flowType_CumulativePackets[flowType] = 0;
    }

    flowType_CumulativePackets.clear();

    flowType_CumulativePackets[flowTypeTag.Get()] += 1;
    lastTime = currentTime;
}

static void
throughputRouterTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),Throughput(Kbps)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static uint64_t cumulativeBits = 0;

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    Ptr<Packet> packet = item->GetPacket();
    uint64_t dequeuedBits = item->GetSize() * 8;

    if (elapsedTime < Time("200ms"))
    {
        cumulativeBits += dequeuedBits;
        return;
    }

    uint64_t throughput = cumulativeBits / elapsedTime.GetSeconds();
    double throughputKbps = double(throughput) / 1e3; // Convert to Kbps

    *stream->GetStream() << currentTime.GetSeconds() << "," << throughputKbps << std::endl;

    cumulativeBits = dequeuedBits;
    lastTime = currentTime;
}

static void
throughputFlowTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,Throughput(Kbps)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativeBits;

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    Ptr<Packet> packet = item->GetPacket();
    uint64_t dequeuedBits = item->GetSize() * 8;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativeBits[flowTypeTag.Get()] += dequeuedBits;
        return;
    }

    for (int i = FlowType::REALTIME_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativeBits = flowType_CumulativeBits[flowType];
        double throughput = double(cumulativeBits) / elapsedTime.GetSeconds();
        double throughputKbps = throughput / 1e3; // Convert to Kbps

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << throughputKbps << std::endl;
    }

    flowType_CumulativeBits.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativeBits[flowTypeTag.Get()] = dequeuedBits;
    lastTime = currentTime;
}

static void
sendingDataRateAppTrace(Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,DataRate(Kbps)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativeBits;

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    uint64_t dequeuedBits = packet->GetSize() * 8;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        packet->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativeBits[flowTypeTag.Get()] += dequeuedBits;
        return;
    }

    for (int i = FlowType::REALTIME_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativeBits = flowType_CumulativeBits[flowType];
        double throughput = double(cumulativeBits) / elapsedTime.GetSeconds();
        double throughputKbps = throughput / 1e3; // Convert to Kbps

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << throughputKbps << std::endl;
    }

    flowType_CumulativeBits.clear();

    FlowTypeTag flowTypeTag;
    packet->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativeBits[flowTypeTag.Get()] = dequeuedBits;
    lastTime = currentTime;
}

static void
receivingDataRateServerTrace(Ptr<OutputStreamWrapper> stream,
                             Ptr<const Packet> packet,
                             const Address& _address)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,DataRate(Kbps)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativeBits;

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    uint64_t dequeuedBits = packet->GetSize() * 8;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        packet->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativeBits[flowTypeTag.Get()] += dequeuedBits;
        return;
    }

    for (int i = FlowType::REALTIME_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t cumulativeBits = flowType_CumulativeBits[flowType];
        double throughput = double(cumulativeBits) / elapsedTime.GetSeconds();
        double throughputKbps = throughput / 1e3; // Convert to Kbps

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << throughputKbps << std::endl;
    }

    flowType_CumulativeBits.clear();

    FlowTypeTag flowTypeTag;
    packet->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativeBits[flowTypeTag.Get()] = dequeuedBits;
    lastTime = currentTime;
}

static void
packetsInQueueFlowTrace(Ptr<OutputStreamWrapper> stream,
                        bool isEnqueue,
                        Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,PacketsInQueue" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_numPackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        if (isEnqueue)
        {
            flowType_numPackets[flowTypeTag.Get()] += 1;
        }
        else
        {
            if (flowType_numPackets[flowTypeTag.Get()] > 0)
            {
                flowType_numPackets[flowTypeTag.Get()] -= 1;
            }
        }
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint32_t packetCount = flowType_numPackets[flowType];
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << packetCount << std::endl;
    }

    flowType_numPackets.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    if (isEnqueue)
    {
        flowType_numPackets[flowTypeTag.Get()] = 1;
    }
    else
    {
        flowType_numPackets[flowTypeTag.Get()] = 0;
    }

    lastTime = currentTime;
}

static void
delayFlowTrace(Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,DelayTime(us)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, Time> flowType_MaxDelayTime{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        EnqueueTimeTag enqueueTimeTag;
        item->GetPacket()->FindFirstMatchingByteTag(enqueueTimeTag);

        Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
        flowType_MaxDelayTime[flowTypeTag.Get()] =
            std::max(flowType_MaxDelayTime[flowTypeTag.Get()], delayTime);

        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);

        uint32_t maxDelayTime = flowType_MaxDelayTime[flowType].GetMicroSeconds();

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << maxDelayTime << std::endl;
    }

    flowType_MaxDelayTime.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    EnqueueTimeTag enqueueTimeTag;
    item->GetPacket()->FindFirstMatchingByteTag(enqueueTimeTag);

    Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
    flowType_MaxDelayTime[flowTypeTag.Get()] =
        std::max(flowType_MaxDelayTime[flowTypeTag.Get()], delayTime);

    lastTime = currentTime;
}

// Log current progress
static void
logCurrentProgress(uint32_t stopTime)
{
    static uint8_t lastCheckpoint = 0;

    double progress = Simulator::Now().GetSeconds() / stopTime * 100.0;

    if (uint8_t(progress) % 5 == 0 && lastCheckpoint != uint8_t(progress))
    {
        lastCheckpoint = uint8_t(progress);
        NS_LOG_UNCOND("✈️ Simulation progress: " << progress << "%");
    }

    Simulator::Schedule(Seconds(5), &logCurrentProgress, stopTime);
}

// ===Main function===
int
main(int argc, char* argv[])
{
    LogComponentEnableAll(LOG_PREFIX_ALL);
    LogComponentEnable("FqCoDelSimulation", LOG_LEVEL_LOGIC);

    std::string BOTTLENECK_BW = "260Mbps";

    NS_LOG_UNCOND("===Get user inputs...===");
    CommandLine cmd;
    cmd.AddValue("bandwidth", "Bandwidth of the bottleneck link", BOTTLENECK_BW);
    cmd.Parse(argc, argv);

    const std::filesystem::path CWD = std::filesystem::current_path();
    const std::string OUTPUT_FOLDER = BOTTLENECK_BW;
    const std::filesystem::path OUTPUT_DIR =
        CWD / "visualization" / "output" / "fq-codel" / OUTPUT_FOLDER;

    NS_LOG_UNCOND("⭐️ Starting simulation...");

    DumbbellTopology topo;
    topo.BuildTopology(BOTTLENECK_BW);

    NS_LOG_UNCOND("===Create queuing mechanism for routers...===");
    TrafficControlHelper tcRouters;
    tcRouters.Uninstall(topo.m_routersDevices.Get(0));
    tcRouters.SetRootQueueDisc("ns3::FqCoDelQueueDisc",
                               "MaxSize",
                               StringValue("1000p"),
                               "Flows",
                               UintegerValue(1024),
                               "Interval",
                               StringValue("100ms"),
                               "Target",
                               StringValue("5ms"));

    QueueDiscContainer queueDiscContainer = tcRouters.Install(topo.m_routersDevices.Get(0));
    Ptr<FqCoDelQueueDisc> experimentQueueDisc =
        queueDiscContainer.Get(0)->GetObject<FqCoDelQueueDisc>();

    NS_LOG_UNCOND("===Config tracing...===");

    // Tracing on experimental router & Write tracing files
    std::filesystem::create_directories(OUTPUT_DIR);
    AsciiTraceHelper ascii;

    // Enqueue tracing
    Ptr<OutputStreamWrapper> flowEnqueueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-enqueue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Enqueue",
        MakeBoundCallback(&enqueueFlowTrace, flowEnqueueStream));

    // Dequeue tracing
    Ptr<OutputStreamWrapper> flowDequeueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-dequeue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&dequeueFlowTrace, flowDequeueStream));

    // Drop tracing
    Ptr<OutputStreamWrapper> flowDropStream = ascii.CreateFileStream(OUTPUT_DIR / "flow-drop.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Drop",
        MakeBoundCallback(&dropFlowTrace, flowDropStream));

    // Throughput tracing
    Ptr<OutputStreamWrapper> routerThroughputStream =
        ascii.CreateFileStream(OUTPUT_DIR / "router-throughput.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&throughputRouterTrace, routerThroughputStream));

    Ptr<OutputStreamWrapper> flowThroughputStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-throughput.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&throughputFlowTrace, flowThroughputStream));

    Ptr<OutputStreamWrapper> appSendingDataRateStream =
        ascii.CreateFileStream(OUTPUT_DIR / "app-sending-data-rate.csv");
    Ptr<OutputStreamWrapper> serverReceivingDataRateStream =
        ascii.CreateFileStream(OUTPUT_DIR / "server-receiving-data-rate.csv");
    for (uint8_t i = 0; i < topo.m_serverApps.GetN(); i++)
    {
        Ptr<MyApplication> clientApplication = topo.m_clientApps.Get(i)->GetObject<MyApplication>();
        clientApplication->TraceConnectWithoutContext(
            "Tx",
            MakeBoundCallback(&sendingDataRateAppTrace, appSendingDataRateStream));

        Ptr<PacketSink> serverApplication = topo.m_serverApps.Get(i)->GetObject<PacketSink>();
        serverApplication->TraceConnectWithoutContext(
            "Rx",
            MakeBoundCallback(&receivingDataRateServerTrace, serverReceivingDataRateStream));
    }

    // Packets in queue tracing
    Ptr<OutputStreamWrapper> packetsInQueueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-packets-in-queue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Enqueue",
        MakeBoundCallback(&packetsInQueueFlowTrace, packetsInQueueStream, true));
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&packetsInQueueFlowTrace, packetsInQueueStream, false));

    // Delay tracing
    Ptr<OutputStreamWrapper> delayStream = ascii.CreateFileStream(OUTPUT_DIR / "flow-delay.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&delayFlowTrace, delayStream));

    // Application tracing
    ApplicationTrace applicationTrace;
    applicationTrace.qosStream = ascii.CreateFileStream(OUTPUT_DIR / "qos.csv");
    applicationTrace.rttStream = ascii.CreateFileStream(OUTPUT_DIR / "rtt.csv");

    applicationTrace.sentReceivedBytesStream =
        ascii.CreateFileStream(OUTPUT_DIR / "sent-received-bytes.csv");
    applicationTrace.retransmissionBytesStream =
        ascii.CreateFileStream(OUTPUT_DIR / "retransmission-bytes.csv");

    applicationTrace.APP_BWS = topo.LEFT_LEAF_APP_BWS;
    applicationTrace.APP_TYPES = topo.LEFT_LEAF_APP_TYPES;
    applicationTrace.LEFT_LEAF_DELAYS = topo.LEFT_LEAF_DELAYS;
    applicationTrace.BOTTLENECK_DELAY = topo.BOTTLENECK_DELAY;
    applicationTrace.TOTAL_APPLICATIONS = topo.NUM_LEAFS;

    for (uint32_t i = 0; i < topo.NUM_LEAFS; i++)
    {
        topo.m_leftLeafDevices.Get(i)->TraceConnectWithoutContext(
            "PhyTxBegin",
            MakeBoundCallback(&ApplicationTrace::ApplicationSentReceivedBytesTrace,
                              &applicationTrace,
                              i,
                              true));
        topo.m_leftLeafDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxEnd",
            MakeBoundCallback(&ApplicationTrace::RTTTrace, &applicationTrace, i, false));

        topo.m_rightLeafDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxEnd",
            MakeBoundCallback(&ApplicationTrace::ApplicationSentReceivedBytesTrace,
                              &applicationTrace,
                              i,
                              false));
        topo.m_rightLeafDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxEnd",
            MakeBoundCallback(&ApplicationTrace::RTTTrace, &applicationTrace, i, true));

        Ptr<MyApplication> clientApplication = topo.m_clientApps.Get(i)->GetObject<MyApplication>();
        clientApplication->TraceConnectWithoutContext(
            "SocketTrace",
            MakeBoundCallback(&ApplicationTrace::SocketTrace, &applicationTrace, i));

        topo.m_rightLeafDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxEnd",
            MakeBoundCallback(&ApplicationTrace::ApplicationThroughputTrace, &applicationTrace, i));
    }

    applicationTrace.delayStream = ascii.CreateFileStream(OUTPUT_DIR / "application-delay.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "Dequeue",
        MakeBoundCallback(&ApplicationTrace::delayTrace, &applicationTrace));

    applicationTrace.Initialize();

    // p2pBottleneck.EnablePcap(OUTPUT_DIR / "capture", routersDevices.Get(0), true);

    NS_LOG_UNCOND("===Starting simulation...===");
    Simulator::Schedule(Seconds(10), &logCurrentProgress, topo.STOP_TIME);

    auto start = std::chrono::high_resolution_clock::now();

    Simulator::Stop(Seconds(topo.STOP_TIME));
    Simulator::Run();
    Simulator::Destroy();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    NS_LOG_UNCOND("✅ Simulation finished -> Duration: " << duration.count() << "s");

    return 0;
}
