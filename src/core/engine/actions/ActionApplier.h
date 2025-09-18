#pragma once

namespace pkt::core
{

class Hand;
struct PlayerAction;

namespace player
{
class Player;
}

class ActionApplier
{
  public:
    static void apply(Hand& hand, const PlayerAction& action);

  private:
    static void applyCallAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory, int currentHighest,
                                int playerBet);
    static void applyRaiseAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                 const PlayerAction& originalAction, int playerBet);
    static void applyBetAction(Hand& hand, player::Player& player, const PlayerAction& action);
    static void applyAllinAction(Hand& hand, player::Player& player, PlayerAction& actionForHistory,
                                 int currentHighest);

    static void updateBetAndPot(Hand& hand, player::Player& player, int amount);
    static void setLastRaiserForCurrentRound(Hand& hand, unsigned playerId);
    static void finalizeAction(Hand& hand, player::Player& player, const PlayerAction& actionForHistory);
};

} // namespace pkt::core
