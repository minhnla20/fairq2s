#ifndef RTQS_QUEUE_DISC_H
#define RTQS_QUEUE_DISC_H

#include "../helper/my-mapping.h"
#include "../tags/application-id-tag.h"
#include "../tags/flow-type-tag.h"
#include "../tags/send-time-tag.h"
#include "../tags/mpd-tag.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/queue-disc.h"

using namespace ns3;

enum RtqsQueueId
{
    RTQS_DETECTION_QUEUE,
    RTQS_REALTIME_QUEUE_1,
    RTQS_REALTIME_QUEUE_2,
    RTQS_REALTIME_QUEUE_3,
    RTQS_NON_REALTIME_QUEUE,
    RTQS_OTHER_QUEUE,
    RTQS_BLACK_HOLE_QUEUE
};

static const std::string RTQS_QUEUE_IDS[] = {"DETECTION_QUEUE",
                                        "REALTIME_QUEUE_1",
                                        "REALTIME_QUEUE_2",
                                        "REALTIME_QUEUE_3",
                                        "NON_REALTIME_QUEUE",
                                        "OTHER_QUEUE",
                                        "BLACK_HOLE_QUEUE"};

class RtqsQueueDisc : public QueueDisc
{
  public:
    static TypeId GetTypeId();
    RtqsQueueDisc();
    ~RtqsQueueDisc() override;

    // Obtain the queueing time of queue: queue_id (estimated by send_packet_deltaT).
    Time GetCurrentQueueingTime(RtqsQueueId queueId);

    // Update ADD and DownMPD when receive ACK packet from the server. Update will happend in router
    // PhyRxEnd event
    void UpdateADDAndDownMPD(uint32_t applicationId, Time newADD, Time newDownMPD);

  private:
    // Base function defined by class QueueDisc.
    virtual bool DoEnqueue(Ptr<QueueDiscItem> item) override;
    virtual Ptr<QueueDiscItem> DoDequeue() override;
    virtual bool CheckConfig() override;
    virtual void InitializeParams() override;

    // Selects the packet from the front of specified queue.
    virtual Ptr<QueueDiscItem> DequeuePacket(RtqsQueueId queueId);

    // ===FLOW STATUS===
    enum FlowStatus
    {
        NOT_STARTED,
        STARTING,
        STABLE
    };

    // Record the status of each flow.
    // This value is updated when the first run for doenqueue for each flow (distingushed by
    // source_id).
    // The value of flow_status is: source_id -> FlowStatus. Flow status inclue three
    // position:
    // 0 - This flow is not start yet.
    // 1 - This flow is just started and may be going through the slow start phase of the congestion
    // control algorithm (not to do max flow reject).
    // 2 - This flow has been started for more than a time window and we consider it to have entered
    // a stable period (maximum flow reject can be performed).
    default_map<uint32_t, FlowStatus> m_flowStatus{FlowStatus::NOT_STARTED};

    void UpdateFlowStatus(uint32_t applicationId, FlowStatus newFlowStatus);

    // To determine which queue this flow should enter by source_id and type_id,
    RtqsQueueId DistributeToQueueId(uint32_t applicationId, FlowType flowType);

    // ===ALL QUEUES===

    default_map<RtqsQueueId, QueueSize> min_queue_size = {
        {RtqsQueueId::RTQS_DETECTION_QUEUE, QueueSize("50p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_1, QueueSize("50p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_2, QueueSize("50p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_3, QueueSize("50p")},
        {RtqsQueueId::RTQS_NON_REALTIME_QUEUE, QueueSize("50p")},
        {RtqsQueueId::RTQS_OTHER_QUEUE, QueueSize("50p")},
        {RtqsQueueId::RTQS_BLACK_HOLE_QUEUE, QueueSize("50p")}};
    default_map<RtqsQueueId, QueueSize> max_queue_size = {
        {RtqsQueueId::RTQS_DETECTION_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_1, QueueSize("1000p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_2, QueueSize("1000p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_3, QueueSize("1000p")},
        {RtqsQueueId::RTQS_NON_REALTIME_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_OTHER_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_BLACK_HOLE_QUEUE, QueueSize("1000p")}};

    // Indicates the current queue length. There are 7 queues in total.
    // 0 - Detection queue. The default queue length is 500.
    // 1~3 - Realtime queue. The default queue length is 500
    // 4 - Non-realtime queue. The default queue length is 1000p
    // 5 - Other queue, the default queue length is 1000p
    // 6 - Black hole queue, the default queue length is 500p
    default_map<RtqsQueueId, QueueSize> m_queueBufferSize{
        {RtqsQueueId::RTQS_DETECTION_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_1, QueueSize("500p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_2, QueueSize("500p")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_3, QueueSize("500p")},
        {RtqsQueueId::RTQS_NON_REALTIME_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_OTHER_QUEUE, QueueSize("1000p")},
        {RtqsQueueId::RTQS_BLACK_HOLE_QUEUE, QueueSize("1000p")}};

    // Update the buffer size for a specific queue
    void SetQueueBufferSize(RtqsQueueId queueId, uint32_t newSize);

    // Obtain the maximum possible queueing time of queue: queue_id (estimated by
    // send_packet_deltaT)
    Time GetMaxQueueingTime(RtqsQueueId queueId);

    // Record the current packets count for each queue, and further record the packets count
    // for each flow (distinguided by source_id).
    // This value is updated when packets enqueue (+1) and dequeue (-1).
    // The value of queue_pkt_count include two level map,
    // the first level is: queue_id -> unorder_map, the second level is source_id -> packets count.
    default_map<RtqsQueueId, default_map<uint32_t, uint32_t>> m_queuePacketCount{0};

    // Obtain current flow count for each queue based on the number of keys in enqueue_throughput,
    // called by setFairBandwidthShare, used to allocate fair bandwidth of different queues.
    uint32_t GetEnqueueFlowCount(RtqsQueueId queueId);

    // Record the fair bandwidth share of each queue.
    // This value will recalculat before each packet is sent (dequeue).
    // The value of fair_bandwidth_share is: queue_id -> fair_bandwidth_share (double).
    // It means the fair share bandwidth of queue: queue_id is: fair_bandwidth_share[queue_id]*100%
    default_map<RtqsQueueId, double> m_fairBandwidthShare{0};

    void UpdateFairBandwidthShare();

    // Polling-based dequeue weight. Note: W
    default_map<RtqsQueueId, double> m_pollingDequeueWeight{0};

    // Polling-based accumulated dequeue value for each queue. Note: V
    default_map<RtqsQueueId, double> m_pollingDequeueValue{0};

    // Polling-based dequeue weight Note: W - V
    default_map<RtqsQueueId, double> m_pollingDequeueFinal{0};

    // Update queue weight params when one queue is dequeued
    void UpdatePollingWeightParams(RtqsQueueId queueId, RtqsQueueId dequeuedRtqsQueueId);

    // ===REALTIME QUEUES===

    // Calculates the average interval between sending two packets in each queue,
    // which is used to calculate the m_point value and control the frequency of queue length
    // increasement. Due to the randomness of packet sending between queues, the average value of
    // the latest send_packet_deltaT_window packet sending interval is selected, noted as
    // send_packet_deltaT. And send_packet_deltaT_sum is the sum of these sending interval,
    // send_packet_deltaT_queue is the sending interval value recorder,
    // send_packet_deltaT_last_record is the timestamp for last recording. These values are updated
    // when packet dequeue. The value of send_packet_deltaT is: queue_id -> Time (average sending
    // interval time). The value of send_packet_deltaT_sum is: queue_id -> Time (sum of latest
    // send_packet_deltaT_window sending interval time). The value of send_packet_deltaT_queue is:
    // queue_id -> Queue<Time> (each sending interval time recorded). The value of
    // send_packet_deltaT_last_record is: queue_id -> Queue<Time> (last record timestamp). That is:
    // send_packet_deltaT[queue_id]=send_packet_deltaT_sum[queue_id]/len(send_packet_deltaT_sum[queue_id]),
    // send_packet_deltaT_sum[queue_id]=sum{send_packet_deltaT_queue[queue_id]},
    // and new_interval_time= Simulator::Now() - send_packet_deltaT_last_record.
    default_map<RtqsQueueId, Time> m_sendPacketDeltaT{Time()};
    default_map<RtqsQueueId, Time> m_sendPacketDeltaTSum{Time()};
    default_map<RtqsQueueId, std::queue<Time>> m_sendPacketDeltaTQueue{};
    default_map<RtqsQueueId, Time> m_sendPacketDeltaTLastRecord{Time()};

    const uint32_t SEND_PACKET_DELTA_T_WINDOW = 10;

    // The underlying method to set/update the value of send_packet_deltaT
    // (as well as send_packet_deltaT_sum and send_packet_deltaT_queue),
    // to make sure the queueing time, m_point calculating and queue length increasement is correct!
    void UpdateSendPacketDeltaT(RtqsQueueId queueId);

    // By default, the queue length increases by 1 every 10ms.
    // Note that, if the value of send_packet_deltaT is not equal to 0, the queue length will
    // increases by 1 after each 10 packets sent or defalut_increase_queue_len_interval, that is,
    // every max(send_packet_deltaT x 10 ms, 10ms); This is because if the current packet sending
    // rate of the application is less than the available bandwidth, send_packet_deltaT may go to 0,
    // which will enter an endless loop to increase queue length.
    const Time DEFAULT_INCREASE_QUEUE_LEN_INTERVAL{"10ms"};

    // Periodically increases the queue size of the real-time queues,
    // called by setAttribute (named queue_len_increase).
    void IncreaseQueueLength(RtqsQueueId queueId);

    // The underlying method to calculate/update the value of m_point,
    // called when doEnqueue, for only realtime flow.
    uint32_t CalculatePoint(Ptr<Packet> packet, RtqsQueueId queueId);

    // To distinguish different realtime queues based on independent delay.
    default_map<RtqsQueueId, Time> m_limitIndependentDelayRealtimeQueueId{
        {RtqsQueueId::RTQS_REALTIME_QUEUE_1, Time("200us")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_2, Time("700us")},
        {RtqsQueueId::RTQS_REALTIME_QUEUE_3, Time("1500us")}};

    // Record the value of MDP for each flow:
    // - UpMPD is the maximum hop delay (MPD) experienced by a flow before it reaches this router,
    // - DownMPD is the maximum hop delay experienced by a flow after it leaves this router to the
    // destination host. How to update:
    // - UpMPD is updated by data Packet sent to destination host
    // - DownMPD is updated by ACK Packet sent back.
    // For flow source_id, the true MPD is max(upMPD[source_id], downMPD[source_id]).
    // The value of both upMPD and downMPD is source_id -> MPD(Time).
    default_map<uint32_t, Time> m_upMPD{Time()};
    default_map<uint32_t, Time> m_downMPD{Time()};

    // Record the value of ADD for each flow.
    default_map<uint32_t, Time> m_minADD{Time()};
    default_map<uint32_t, Time> m_currentADD{Time()};

    // Record the value of ADU for each flow.
    default_map<uint32_t, Time> m_minADU{Time()};
    default_map<uint32_t, Time> m_currentADU{Time()};

    // Latency requirements for real-time streams.
    // We believe that the total queue time should be less than this value.
    // => ADU + ADD + queueing_time < DELAY_INTERVAL_THRESHOLD.
    const Time DELAY_INTERVAL_THRESHOLD{Time("1500us")};

    // ===TRACING===
    /// Traced callback: fired when a packet is enqueued
    TracedCallback<Ptr<RtqsQueueDisc>, RtqsQueueId, Ptr<QueueDiscItem>> m_traceRtqsEnqueue;
    /// Traced callback: fired when a packet is dequeued
    TracedCallback<Ptr<RtqsQueueDisc>, RtqsQueueId, Ptr<QueueDiscItem>> m_traceRtqsDequeue;
    /// Traced callback: fired when a packet is dropped
    TracedCallback<Ptr<RtqsQueueDisc>, RtqsQueueId, Ptr<QueueDiscItem>> m_traceRtqsDrop;
};

#endif // RTQS_QUEUE_DISC_H