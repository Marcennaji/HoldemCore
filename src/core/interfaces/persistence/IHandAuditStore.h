#pragma once
#include <string>

class Hand;
class Range;

namespace pkt::core
{

class IHandAuditStore
{
  public:
    virtual ~IHandAuditStore() = default;

    virtual void updateUnplausibleHand(const std::string card1, const std::string card2, const bool human,
                                       const char bettingRound, const int nbPlayers) = 0;
};
} // namespace pkt::core
