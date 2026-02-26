#ifndef DUMBBELL_TOPOLOGY_H
#define DUMBBELL_TOPOLOGY_H

#include "../applications/my-application.h"
#include "../helper/my-mapping.h"
#include "../tags/application-id-tag.h"
#include "../tags/enqueue-time-tag.h"
#include "../tags/flow-type-tag.h"
#include "../tags/send-time-tag.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include <string>
#include <tuple>
#include <vector>

namespace ns3
{

/**
 * \ingroup my-custom
 * \brief Set up a standard dumbbell topology for simulation experiments.
 */
class DumbbellTopology
{
  public:
    DumbbellTopology();
    ~DumbbellTopology() = default;

    /**
     * \brief Build nodes, links, devices, internet stack, TCP configurations, IP addresses, and
     * routing tables.
     *
     * \param bottleneckBw Bottleneck link bandwidth string (e.g., "178Mbps").
     */
    void BuildTopology(const std::string& bottleneckBw = "260Mbps");

    const uint32_t STOP_TIME{1}; // seconds

    const uint32_t NUM_ROUTERS{2};
    const uint32_t NUM_REALTIME_FLOWS{50};
    const uint32_t NUM_NON_REALTIME_FLOWS{50};
    const uint32_t NUM_OTHER_FLOWS{50};
    const uint32_t NUM_LEAFS{NUM_REALTIME_FLOWS + NUM_NON_REALTIME_FLOWS + NUM_OTHER_FLOWS};

    const std::vector<Time> SCHEDULED_START_TIMES = {Seconds(0),
                                                     Seconds(50),
                                                     Seconds(100),
                                                     Seconds(150),
                                                     Seconds(200),
                                                     Seconds(250),
                                                     Seconds(300)};
    const std::vector<uint32_t> CONCURRENT_WORKING_REALTIME_FLOWS_COUNT{10, 20, 20, 50, 20, 20, 10};
    const std::vector<uint32_t>
        CONCURRENT_WORKING_NON_REALTIME_FLOWS_COUNT{10, 10, 40, 50, 40, 10, 10};
    const std::vector<uint32_t> CONCURRENT_WORKING_OTHER_FLOWS_COUNT{10, 15, 30, 50, 30, 15, 10};

    std::string m_bottleneckBw{"260Mbps"};
    const uint32_t BOTTLENECK_MTU{1500};
    const std::string BOTTLENECK_DELAY{"1us"};

    std::vector<FlowType> LEFT_LEAF_APP_TYPES{};
    std::vector<std::string> LEFT_LEAF_APP_BWS{};
    std::vector<std::string> LEFT_LEAF_DELAYS{};
    std::vector<std::string> RIGHT_LEAF_DELAYS{};
    std::vector<uint32_t> LEFT_LEAF_APP_PACKET_SIZES{};
    default_map<uint32_t, std::pair<Time, Time>> APPLICATION_SCHEDULED_TIMES{
        {Seconds(-1), Seconds(-1)}}; // applicationId -> (startTime, stopTime)

    ApplicationContainer m_clientApps{};
    ApplicationContainer m_serverApps{};

    NodeContainer m_routers;
    NodeContainer m_leftLeafs;
    NodeContainer m_rightLeafs;

    NetDeviceContainer m_routersDevices;
    NetDeviceContainer m_leftLeafDevices;
    NetDeviceContainer m_rightLeafDevices;
    NetDeviceContainer m_leftRouterDevices;
    NetDeviceContainer m_rightRouterDevices;

    Ipv4InterfaceContainer m_routerInterfaces;
    Ipv4InterfaceContainer m_leftLeafInterfaces;
    Ipv4InterfaceContainer m_leftRouterInterfaces;
    Ipv4InterfaceContainer m_rightLeafInterfaces;
    Ipv4InterfaceContainer m_rightRouterInterfaces;
};

} // namespace ns3

#endif // DUMBBELL_TOPOLOGY_H
