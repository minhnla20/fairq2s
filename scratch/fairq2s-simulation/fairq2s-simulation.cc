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

NS_LOG_COMPONENT_DEFINE("FairQ2SSimulation");

// ===Global variables===

// ===Tracing functions===

// Trace network metrics
void
enqueueQueueTrace(Ptr<OutputStreamWrapper> stream,
                  Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                  FairQ2SQueueId queueId,
                  Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FairQ2SQueueId, uint64_t> queueId_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        queueId_CumulativePackets[queueId] += 1;
        return;
    }

    for (uint8_t i = FairQ2SQueueId::FAIRQ2S_REALTIME_QUEUE;
         i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << queueId_CumulativePackets[queueId] << std::endl;
    }

    queueId_CumulativePackets.clear();

    queueId_CumulativePackets[queueId] = 1;
    lastTime = currentTime;
}

void
enqueueFlowTrace(Ptr<OutputStreamWrapper> stream,
                 Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                 FairQ2SQueueId queueId,
                 Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << flowType_CumulativePackets[flowType] << std::endl;
    }

    flowType_CumulativePackets.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativePackets[flowTypeTag.Get()] = 1;
    lastTime = currentTime;
}

void
dequeueQueueTrace(Ptr<OutputStreamWrapper> stream,
                  Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                  FairQ2SQueueId queueId,
                  Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FairQ2SQueueId, uint64_t> queueId_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        queueId_CumulativePackets[queueId] += 1;
        return;
    }

    for (uint8_t i = FairQ2SQueueId::FAIRQ2S_REALTIME_QUEUE;
         i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << queueId_CumulativePackets[queueId] << std::endl;
    }

    queueId_CumulativePackets.clear();

    queueId_CumulativePackets[queueId] = 1;
    lastTime = currentTime;
}

void
dequeueFlowTrace(Ptr<OutputStreamWrapper> stream,
                 Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                 FairQ2SQueueId queueId,
                 Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (uint8_t i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << flowType_CumulativePackets[flowType] << std::endl;
    }

    flowType_CumulativePackets.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativePackets[flowTypeTag.Get()] = 1;
    lastTime = currentTime;
}

void
dropQueueTrace(Ptr<OutputStreamWrapper> stream,
               Ptr<FairQ2SQueueDisc> fairQ2SQueue,
               FairQ2SQueueId queueId,
               Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FairQ2SQueueId, uint64_t> queueId_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        queueId_CumulativePackets[queueId] += 1;
        return;
    }

    for (uint8_t i = FairQ2SQueueId::FAIRQ2S_REALTIME_QUEUE;
         i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << queueId_CumulativePackets[queueId] << std::endl;
    }

    queueId_CumulativePackets.clear();

    queueId_CumulativePackets[queueId] = 1;
    lastTime = currentTime;
}

void
dropFlowTrace(Ptr<OutputStreamWrapper> stream,
              Ptr<FairQ2SQueueDisc> fairQ2SQueue,
              FairQ2SQueueId queueId,
              Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,NumberPackets" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FlowType, uint32_t> flowType_CumulativePackets{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        FlowTypeTag flowTypeTag;
        item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

        flowType_CumulativePackets[flowTypeTag.Get()] += 1;
        return;
    }

    for (int i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << flowType_CumulativePackets[flowType] << std::endl;
    }

    flowType_CumulativePackets.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativePackets[flowTypeTag.Get()] = 1;
    lastTime = currentTime;
}

void
throughputRouterTrace(Ptr<OutputStreamWrapper> stream,
                      Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                      FairQ2SQueueId queueId,
                      Ptr<QueueDiscItem> item)
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

void
throughputQueueTrace(Ptr<OutputStreamWrapper> stream,
                     Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                     FairQ2SQueueId queueId,
                     Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,Throughput(Kbps)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FairQ2SQueueId, uint32_t> queueId_CumulativeBits;

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    Ptr<Packet> packet = item->GetPacket();
    uint64_t dequeuedBits = item->GetSize() * 8;

    if (elapsedTime < Time("200ms"))
    {
        queueId_CumulativeBits[queueId] += dequeuedBits;
        return;
    }

    for (int i = FairQ2SQueueId::FAIRQ2S_DETECTION_QUEUE; i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        uint64_t throughput = queueId_CumulativeBits[queueId] / elapsedTime.GetSeconds();
        double throughputKbps = double(throughput) / 1e3; // Convert to Kbps

        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << throughputKbps << std::endl;
    }

    queueId_CumulativeBits.clear();

    queueId_CumulativeBits[queueId] = dequeuedBits;
    lastTime = currentTime;
}

void
throughputFlowTrace(Ptr<OutputStreamWrapper> stream,
                    Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                    FairQ2SQueueId queueId,
                    Ptr<QueueDiscItem> item)
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

    for (int i = FlowType::HANDSHAKE_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint64_t throughput = flowType_CumulativeBits[flowType] / elapsedTime.GetSeconds();
        double throughputKbps = double(throughput) / 1e3; // Convert to Kbps

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << throughputKbps << std::endl;
    }

    flowType_CumulativeBits.clear();

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    flowType_CumulativeBits[flowTypeTag.Get()] = dequeuedBits;
    lastTime = currentTime;
}

void
delayFlowTrace(Ptr<OutputStreamWrapper> stream,
               Ptr<FairQ2SQueueDisc> fairQ2SQueue,
               FairQ2SQueueId queueId,
               Ptr<QueueDiscItem> item)
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

    FlowTypeTag flowTypeTag;
    item->GetPacket()->FindFirstMatchingByteTag(flowTypeTag);

    EnqueueTimeTag enqueueTimeTag;
    item->GetPacket()->FindFirstMatchingByteTag(enqueueTimeTag);

    if (elapsedTime < Time("200ms"))
    {
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

    Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
    flowType_MaxDelayTime[flowTypeTag.Get()] =
        std::max(flowType_MaxDelayTime[flowTypeTag.Get()], delayTime);

    lastTime = currentTime;
}

void
delayQueueTrace(Ptr<OutputStreamWrapper> stream,
                Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                FairQ2SQueueId queueId,
                Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,DelayTime(us)" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);
    static default_map<FairQ2SQueueId, Time> queueId_MaxDelayTime{0};

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    EnqueueTimeTag enqueueTimeTag;
    item->GetPacket()->FindFirstMatchingByteTag(enqueueTimeTag);

    if (elapsedTime < Time("200ms"))
    {
        Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
        queueId_MaxDelayTime[queueId] = std::max(queueId_MaxDelayTime[queueId], delayTime);

        return;
    }

    for (int i = FairQ2SQueueId::FAIRQ2S_DETECTION_QUEUE; i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        uint32_t maxDelayTime = queueId_MaxDelayTime[queueId].GetMicroSeconds();

        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << maxDelayTime << std::endl;
    }

    Time delayTime = Simulator::Now() - enqueueTimeTag.Get();
    queueId_MaxDelayTime[queueId] = std::max(queueId_MaxDelayTime[queueId], delayTime);

    lastTime = currentTime;
}

void
packetsInQueueQueueTrace(Ptr<OutputStreamWrapper> stream,
                         Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                         FairQ2SQueueId queueId,
                         Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),QueueId,PacketsInQueue" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        // Waiting for more than 1s
        return;
    }

    for (int i = FairQ2SQueueId::FAIRQ2S_DETECTION_QUEUE; i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        uint32_t numPackets = fairQ2SQueue->GetInternalQueue(queueId)->GetCurrentSize().GetValue();

        *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                             << numPackets << std::endl;
    }

    lastTime = currentTime;
}

void
packetsInQueueFlowTrace(Ptr<OutputStreamWrapper> stream,
                        Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                        FairQ2SQueueId queueId,
                        Ptr<QueueDiscItem> item)
{
    static bool isOk = [stream]() {
        *stream->GetStream() << "Time(s),FlowType,PacketsInQueue" << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    static Time lastTime = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastTime;

    if (elapsedTime < Time("200ms"))
    {
        // Waiting for more than 1s
        return;
    }

    default_map<FlowType, uint32_t> flowType_CumulativePacketsInQueue;

    for (int i = FairQ2SQueueId::FAIRQ2S_DETECTION_QUEUE; i <= FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE;
         ++i)
    {
        FairQ2SQueueId queueId = static_cast<FairQ2SQueueId>(i);
        uint32_t numPackets = fairQ2SQueue->GetInternalQueue(queueId)->GetCurrentSize().GetValue();

        switch (queueId)
        {
        case FairQ2SQueueId::FAIRQ2S_REALTIME_QUEUE:
            flowType_CumulativePacketsInQueue[FlowType::REALTIME_FLOW] += numPackets;
            break;

        case FairQ2SQueueId::FAIRQ2S_NON_REALTIME_QUEUE:
            flowType_CumulativePacketsInQueue[FlowType::NON_REALTIME_FLOW] += numPackets;
            break;

        case FairQ2SQueueId::FAIRQ2S_OTHER_QUEUE:
            flowType_CumulativePacketsInQueue[FlowType::OTHER_FLOW] += numPackets;
            break;

        default:
            break;
        }
    }

    for (int i = FlowType::REALTIME_FLOW; i <= FlowType::OTHER_FLOW; ++i)
    {
        FlowType flowType = static_cast<FlowType>(i);
        uint32_t numPackets = flowType_CumulativePacketsInQueue[flowType];

        *stream->GetStream() << currentTime.GetSeconds() << "," << FLOW_TYPES[flowType] << ","
                             << numPackets << std::endl;
    }

    lastTime = currentTime;
}

void
delayApplicationTrace(ApplicationTrace* applicationTrace,
                      Ptr<FairQ2SQueueDisc> fairQ2SQueue,
                      FairQ2SQueueId queueId,
                      Ptr<QueueDiscItem> item)
{
    applicationTrace->delayTrace(applicationTrace, item);
}

void
writeReportTrace(Ptr<OutputStreamWrapper> stream, std::string content, bool forceWrite = false)
{
    static Time lastCheckpoint = Time(0);

    Time currentTime = Simulator::Now();
    Time elapsedTime = currentTime - lastCheckpoint;

    if (elapsedTime > Seconds(5) || forceWrite)
    {
        lastCheckpoint = currentTime;
        *stream->GetStream() << "=== At time " << currentTime.GetSeconds() << "s ===" << std::endl;

        *stream->GetStream() << content << std::endl << std::endl;
    }
}

// std::queue<double> log_NormalBuffers; // KB
// std::queue<double> log_EnqueueThroughputs; // Kbps
// std::queue<double> log_FutureBuffers; // KB
// std::queue<double> log_BufferWeights; // 0-1
// std::queue<double> log_NormalizedBufferWeights; // 0-1
// std::queue<double> log_CurrentWaitingTimes; // us
// std::queue<double> log_FutureWaitingTimes; // us
// std::queue<double> log_DelayWeights; // 0-1
// std::queue<double> log_NormalizedDelayWeights; // 0-1
// std::queue<double> log_CombinationWeights; // 0-1
// std::queue<double> log_ScheduledDequeueBuffers; // KB
void
logTrace(Ptr<OutputStreamWrapper> stream,
         FairQ2SQueueId queueId,
         double normalBuffer,
         double enqueueThroughput,
         double futureBuffer,
         double bufferWeight,
         double normalizedBufferWeight,
         double currentWaitingTime,
         double futureWaitingTime,
         double delayWeight,
         double normalizedDelayWeight,
         double combinationWeight,
         double scheduledDequeueBuffer)
{
    static bool isOk = [stream]() {
        *stream->GetStream()
            << "Time(s),QueueId,NormalBuffer(KB),EnqueueThroughput(Kbps),FutureBuffer(KB),"
               "BufferWeight,NormalizedBufferWeight,"
               "CurrentWaitingTime(us),FutureWaitingTime(us),DelayWeight,"
               "NormalizedDelayWeight,CombinationWeight,ScheduledDequeueBuffer(KB)"
            << std::endl;
        return true;
    }();
    NS_ASSERT(isOk);

    Time currentTime = Simulator::Now();
    *stream->GetStream() << currentTime.GetSeconds() << "," << FAIRQ2S_QUEUE_IDS[queueId] << ","
                         << normalBuffer << "," << enqueueThroughput << "," << futureBuffer << ","
                         << bufferWeight << "," << normalizedBufferWeight << ","
                         << currentWaitingTime << "," << futureWaitingTime << "," << delayWeight
                         << "," << normalizedDelayWeight << "," << combinationWeight << ","
                         << scheduledDequeueBuffer << std::endl;
}

// Log current progress
void
logCurrentProgress(uint32_t stopTime)
{
    static uint8_t lastCheckpoint = 0;

    double progress = Simulator::Now().GetSeconds() / stopTime * 100.0;

    if (uint8_t(progress) % 5 == 0 && lastCheckpoint != uint8_t(progress))
    {
        lastCheckpoint = uint8_t(progress);
        NS_LOG_UNCOND("✈️ Simulation progress: " << progress << "%");
    }

    Simulator::Schedule(Seconds(10), &logCurrentProgress, stopTime);
}

// ===Main function===
int
main(int argc, char* argv[])
{
    LogComponentEnableAll(LOG_PREFIX_ALL);
    LogComponentEnable("FairQ2SSimulation", LOG_LEVEL_LOGIC);
    // LogComponentEnable("PacketSink", LOG_LEVEL_LOGIC);
    // LogComponentEnable("RealtimeApplication", LOG_LEVEL_LOGIC);

    std::string BOTTLENECK_BW = "260Mbps";
    double ALPHA = 0.5;

    NS_LOG_UNCOND("===Get user inputs...===");
    CommandLine cmd;
    cmd.AddValue("bandwidth", "Bandwidth of the bottleneck link", BOTTLENECK_BW);
    cmd.AddValue("alpha",
                 "Alpha value for Buffer weight in the combined weight calculation",
                 ALPHA);
    cmd.Parse(argc, argv);

    static const std::filesystem::path CWD = std::filesystem::current_path();
    static const std::string EXPERIMENT_NAME = "fairq2s_" + std::to_string(ALPHA);
    static const std::string OUTPUT_FOLDER = BOTTLENECK_BW;
    static const std::filesystem::path OUTPUT_DIR =
        CWD / "visualization" / "output" / EXPERIMENT_NAME / OUTPUT_FOLDER;

    NS_LOG_UNCOND("⭐️ Starting simulation...");
    NS_LOG_UNCOND("Running experiment with the configured parameters:");
    NS_LOG_UNCOND("Bottleneck bandwidth: " << BOTTLENECK_BW);
    NS_LOG_UNCOND("Alpha: " << ALPHA);

    // All the network is defined in the Topology class
    DumbbellTopology topo;
    topo.BuildTopology(BOTTLENECK_BW);

    NS_LOG_UNCOND("===Create queuing mechanism for routers...===");
    TrafficControlHelper tcRouters;
    tcRouters.Uninstall(topo.m_routersDevices.Get(0));
    tcRouters.SetRootQueueDisc("ns3::FairQ2SQueueDisc", "BufferWeightAlpha", DoubleValue(ALPHA));

    QueueDiscContainer queueDiscContainer = tcRouters.Install(topo.m_routersDevices.Get(0));
    Ptr<FairQ2SQueueDisc> experimentQueueDisc =
        queueDiscContainer.Get(0)->GetObject<FairQ2SQueueDisc>();

    NS_LOG_UNCOND("===Config tracing...===");

    // Tracing on experimental router & Write tracing files
    std::filesystem::create_directories(OUTPUT_DIR);
    AsciiTraceHelper ascii;

    // Enqueue tracing
    Ptr<OutputStreamWrapper> queueEnqueueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-enqueue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SEnqueue",
        MakeBoundCallback(&enqueueQueueTrace, queueEnqueueStream));

    Ptr<OutputStreamWrapper> flowEnqueueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-enqueue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SEnqueue",
        MakeBoundCallback(&enqueueFlowTrace, flowEnqueueStream));

    // Dequeue tracing
    Ptr<OutputStreamWrapper> queueDequeueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-dequeue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&dequeueQueueTrace, queueDequeueStream));

    Ptr<OutputStreamWrapper> flowDequeueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-dequeue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&dequeueFlowTrace, flowDequeueStream));

    // Drop tracing
    Ptr<OutputStreamWrapper> queueDropStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-drop.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDrop",
        MakeBoundCallback(&dropQueueTrace, queueDropStream));

    Ptr<OutputStreamWrapper> flowDropStream = ascii.CreateFileStream(OUTPUT_DIR / "flow-drop.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDrop",
        MakeBoundCallback(&dropFlowTrace, flowDropStream));

    // Throughput tracing
    Ptr<OutputStreamWrapper> routerThroughputStream =
        ascii.CreateFileStream(OUTPUT_DIR / "router-throughput.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&throughputRouterTrace, routerThroughputStream));

    Ptr<OutputStreamWrapper> queueThroughputStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-throughput.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&throughputQueueTrace, queueThroughputStream));

    Ptr<OutputStreamWrapper> flowThroughputStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-throughput.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&throughputFlowTrace, flowThroughputStream));

    // Packets in queue tracing
    Ptr<OutputStreamWrapper> packetsInQueueQueueStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-packets-in-queue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SEnqueue",
        MakeBoundCallback(&packetsInQueueQueueTrace, packetsInQueueQueueStream));
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&packetsInQueueQueueTrace, packetsInQueueQueueStream));

    Ptr<OutputStreamWrapper> packetsInQueueFlowStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-packets-in-queue.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SEnqueue",
        MakeBoundCallback(&packetsInQueueFlowTrace, packetsInQueueFlowStream));
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&packetsInQueueFlowTrace, packetsInQueueFlowStream));

    // Delay tracing
    Ptr<OutputStreamWrapper> queueDelayStream =
        ascii.CreateFileStream(OUTPUT_DIR / "queue-delay.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&delayQueueTrace, queueDelayStream));

    Ptr<OutputStreamWrapper> flowDelayStream =
        ascii.CreateFileStream(OUTPUT_DIR / "flow-delay.csv");
    experimentQueueDisc->TraceConnectWithoutContext(
        "FairQ2SDequeue",
        MakeBoundCallback(&delayFlowTrace, flowDelayStream));

    // Ptr<OutputStreamWrapper> logStream = ascii.CreateFileStream(OUTPUT_DIR / "log.csv");
    // experimentQueueDisc->TraceConnectWithoutContext("FairQ2SLog",
    //                                                 MakeBoundCallback(&logTrace, logStream));

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
        "FairQ2SDequeue",
        MakeBoundCallback(&delayApplicationTrace, &applicationTrace));

    applicationTrace.Initialize();

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
