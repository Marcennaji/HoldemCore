#pragma once

#include "IBettingRoundState.h"
#include "core/engine/Hand.h"

namespace pkt::core
{

class PreflopState : public IBettingRoundState
{
  public:
    void enter(Hand& hand) override
    {
        // hand.dealHoleCards();
        // hand.postBlinds();
        // hand.startBettingRound();
    }
};

} // namespace pkt::core
