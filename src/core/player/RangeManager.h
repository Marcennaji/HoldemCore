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

    // convert a range into a list of real cards, or a list of simplified ranges
    // if returnRange is true, the range is processed and split, only to remove the + signs (if any)
    // otherwise, if returnRange is false, it returns the atomic values of the range (the "real cards")
    static std::vector<std::string> getRangeAtomicValues(const std::string& ranges, const bool returnRange = false);

    void computeEstimatedPreflopRange(CurrentHandContext&);
    int getStandardRaisingRange(int nbPlayers) const;
    int getStandardCallingRange(int nbPlayers) const;
    std::string substractRange(const std::string& originRanges, const std::string& rangesToSubstract,
                               const std::string& board = "") const;

    static std::string getStringRange(int nbPlayers, int range);

    void updateUnplausibleRangesGivenPreflopActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenFlopActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenTurnActions(CurrentHandContext&);
    void updateUnplausibleRangesGivenRiverActions(CurrentHandContext&);

  private:
    std::string computeEstimatedPreflopRangeFromLastRaiser(CurrentHandContext&) const;
    std::string computeEstimatedPreflopRangeFromCaller(CurrentHandContext&) const;

    std::string getFilledRange(std::vector<std::string>& ranges, std::vector<float>& rangesValues,
                               const float rangeMax) const;

    // getRangeAtomicValues helpers :
    static void handleExactPair(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleThreeCharRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleFourCharRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleSuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                  std::vector<std::string>& result);
    static void handleOffsuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                     std::vector<std::string>& result);
    static void handlePairAndAboveRange(char c, const bool returnRange, std::vector<std::string>& result);
    static void handleOffsuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                             std::vector<std::string>& result);
    static void handleSuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                          std::vector<std::string>& result);
    static void handleExactPairRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static char incrementCardValue(char c);

    std::string myEstimatedRange;
    int myPlayerId;
    IHand* myHand;
    IPlayersStatisticsStore* myStatsStore;
};
} // namespace pkt::core::player