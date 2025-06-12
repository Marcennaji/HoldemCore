
#pragma once

#include <core/player/Player.h>
#include <core/player/strategy/PreflopRangeCalculator.h>

namespace pkt::core::player
{
// Forward declaration
struct CurrentHandContext;

class IBotStrategy
{
  public:
    IBotStrategy() : preflopRangeCalculator(std::make_unique<PreflopRangeCalculator>()) {}

    virtual bool preflopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool flopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool turnShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool riverShouldCall(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int preflopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int flopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int flopShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldBet(CurrentHandContext&, bool deterministic = false) = 0;

    const std::string& getStrategyName() const { return myStrategyName; }
    void setStrategyName(const std::string& name) { myStrategyName = name; }
    std::unique_ptr<PreflopRangeCalculator>& getPreflopRangeCalculator() { return preflopRangeCalculator; }

    virtual ~IBotStrategy() = default;

  protected:
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);

    float getPreflopRaisingRange(CurrentHandContext&, bool deterministic = false) const;
    int getRange(PlayerPosition p, int nbPlayers) const;
    int computePreflopRaiseAmount(CurrentHandContext&, bool deterministic = false);
    bool shouldPotControl(CurrentHandContext&, bool deterministic = false);

    bool myShouldCall = false;
    bool myShouldRaise = false;
    std::string myStrategyName;

  private:
    std::unique_ptr<PreflopRangeCalculator> preflopRangeCalculator;

    // vector index is player position, value is range %
    std::vector<int> UTG_STARTING_RANGE;
    std::vector<int> UTG_PLUS_ONE_STARTING_RANGE;
    std::vector<int> UTG_PLUS_TWO_STARTING_RANGE;
    std::vector<int> MIDDLE_STARTING_RANGE;
    std::vector<int> MIDDLE_PLUS_ONE_STARTING_RANGE;
    std::vector<int> LATE_STARTING_RANGE;
    std::vector<int> CUTOFF_STARTING_RANGE;
    std::vector<int> BUTTON_STARTING_RANGE;
    std::vector<int> SB_STARTING_RANGE;
    std::vector<int> BB_STARTING_RANGE;
};
} // namespace pkt::core::player
