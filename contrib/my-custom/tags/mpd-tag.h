#ifndef MPD_TAG_H
#define MPD_TAG_H

#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/nstime.h"

using namespace ns3;

class MPDTag : public Tag
{
  public:
    MPDTag();
    ~MPDTag();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;
    virtual uint32_t GetSerializedSize() const;
    virtual void Serialize(TagBuffer i) const;
    virtual void Deserialize(TagBuffer i);
    virtual void Print(std::ostream& os) const;

    Time Get() const;

  private:
    Time m_mpd{0};
};

#endif // MPD_TAG_H