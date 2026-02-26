#include "mpd-tag.h"

using namespace ns3;

MPDTag::MPDTag()
{
}

MPDTag::~MPDTag()
{
}

TypeId
MPDTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::MPDTag")
                            .SetParent<Tag>()
                            .SetGroupName("Tags")
                            .AddConstructor<MPDTag>()
                            .AddAttribute("MPD",
                                          "Per-hop delay value",
                                          EmptyAttributeValue(),
                                          MakeTimeAccessor(&MPDTag::m_mpd),
                                          MakeTimeChecker());
    return tid;
}

TypeId
MPDTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
MPDTag::GetSerializedSize() const
{
    return sizeof(int64_t);
}

void
MPDTag::Serialize(TagBuffer i) const
{
    i.WriteU64(m_mpd.GetNanoSeconds());
}

void
MPDTag::Deserialize(TagBuffer i)
{
    m_mpd = NanoSeconds(i.ReadU64());
}

void
MPDTag::Print(std::ostream& os) const
{
    os << "MPD=" << m_mpd.GetSeconds() << "s";
}

Time
MPDTag::Get() const
{
    return m_mpd;
}