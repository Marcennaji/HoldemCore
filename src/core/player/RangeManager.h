#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/PlayerStatistics.h>
#include <memory>
#include <string>
#include <vector>

struct PostFlopState;

namespace pkt::core
{
class IHand;
class IPlayersStatisticsStore;

struct PreflopStatistics;
struct FlopStatistics;
struct TurnStatistics;
struct RiverStatistics;
} // namespace pkt::core

namespace pkt::core::player
{
class Player;
struct CurrentHandContext;

class RangeManager
{
  public:
    RangeManager(int playerId, IPlayersStatisticsStore* statsStore);

    void setHand(IHand* hand) { myHand = hand; }
    void setEstimatedRange(const std::string& range);
    std::string getEstimatedRange() const;
    std::vector<std::string> getRangeAtomicValues(std::string range, const bool returnRange = false) const;
    void computeEstimatedPreflopRange(CurrentHandContext&);
    int getStandardRaisingRange(int nbPlayers) const;
    int getStandardCallingRange(int nbPlayers) const;
    std::string substractRange(const std::string startingRange, const std::string rangeToSubstract,
                               const std::string board = "");
    std::string getHandToRange(const std::string card1, const std::string card2) const;
    static std::string getStringRange(int nbPlayers, int range);

    void updateUnplausibleRangesGivenPreflopActions(CurrentHandContext& context);
    void updateUnplausibleRangesGivenFlopActions(CurrentHandContext& context);
    void updateUnplausibleRangesGivenTurnActions(CurrentHandContext& context);
    void updateUnplausibleRangesGivenRiverActions(CurrentHandContext& context);

  private:
    std::string computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext& ctx) const;
    std::string computeEstimatedPreflopRangeFromCaller(CurrentHandContext& ctx) const;
    char incrementCardValue(char c) const;
    std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                               const float rangeMax) const;

    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
    IPlayersStatisticsStore* myStatsStore;
};
} // namespace pkt::core::player