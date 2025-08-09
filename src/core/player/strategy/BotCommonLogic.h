#pragma once

namespace pkt::core::player
{
struct CurrentHandContext;

class BotCommonLogic
{
  protected:
    int computePreflopRaiseAmount(const CurrentHandContext&);
    bool shouldPotControl(const CurrentHandContext&);
    bool myShouldCall;
    bool myShouldRaise;

  private:
    int computeFirstRaiseAmount(const CurrentHandContext&, int bigBlind) const;
    void adjustRaiseForPosition(const CurrentHandContext&, int& raiseAmount, int bigBlind) const;
    void adjustRaiseForLimpers(const CurrentHandContext&, int& raiseAmount, int bigBlind) const;
    int computeReRaiseAmount(const CurrentHandContext&, int bigBlind) const;
    int computeThreeBetAmount(const CurrentHandContext&, int totalPot) const;
    int computeFourBetOrMoreAmount(const CurrentHandContext&, int totalPot) const;
    int finalizeRaiseAmount(const CurrentHandContext&, int raiseAmount) const;

    bool shouldPotControlForPocketPair(const CurrentHandContext&) const;
    bool shouldPotControlForFullHousePossibility(const CurrentHandContext&) const;
    bool shouldPotControlOnFlop(const CurrentHandContext&, int bigBlind) const;
    bool shouldPotControlOnTurn(const CurrentHandContext&, int bigBlind) const;

    void logPotControl() const;
};
} // namespace pkt::core::player
