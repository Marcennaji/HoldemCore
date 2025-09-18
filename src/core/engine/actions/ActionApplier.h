#pragma once

namespace pkt::core
{

class Hand;
struct PlayerAction;

class ActionApplier
{
  public:
    static void apply(Hand& hand, const PlayerAction& action);
};

} // namespace pkt::core
