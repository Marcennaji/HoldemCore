#include "PsimHandEvaluationEngine.h"
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/services/ServiceContainer.h"
#include "third_party/psim/psim.hpp"

namespace pkt::infra
{

using namespace pkt::core;
using namespace std;

PsimHandEvaluationEngine::PsimHandEvaluationEngine()
{
    // Will be lazily initialized
}

PsimHandEvaluationEngine::PsimHandEvaluationEngine(std::shared_ptr<pkt::core::ServiceContainer> services)
    : myServices(std::move(services))
{
}

void PsimHandEvaluationEngine::ensureServicesInitialized()
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

unsigned int PsimHandEvaluationEngine::rankHand(const char* hand)
{
    unsigned int result = ::RankHand(hand);
    return result;
}
HandSimulationStats PsimHandEvaluationEngine::convertSimResults(const SimResults& s)
{
    return {s.win,    s.tie,  s.lose, s.winRanged, s.winSd,      s.tieSd,
            s.loseSd, s.dNow, s.d94,  s.d90,       s.evaluations

    };
}
PostFlopAnalysisFlags PsimHandEvaluationEngine::convertPostFlopState(const PostFlopState& src)
{
    return {src.UsesFirst,
            src.UsesSecond,
            src.IsNoPair,
            src.IsOnePair,
            src.IsPocketPair,
            src.IsTwoPair,
            src.IsTrips,
            src.IsStraight,
            src.IsFlush,
            src.IsFullHouse,
            src.IsQuads,
            src.IsStFlush,
            src.IsTopPair,
            src.IsMiddlePair,
            src.IsBottomPair,
            src.IsOverPair,
            src.IsOverCards,
            src.IsStraightDrawPossible,
            src.IsStraightPossible,
            src.IsFlushDrawPossible,
            src.IsFlushPossible,
            src.IsFullHousePossible,
            src.Is3Flush,
            src.Is4Flush,
            src.FlushOuts,
            src.StraightOuts,
            src.BetterOuts};
}
HandSimulationStats PsimHandEvaluationEngine::simulateHandEquity(const std::string& hand, const std::string& board,
                                                                 const int nbOpponents, float maxOpponentsStrengths)
{
    ensureServicesInitialized();

    SimResults r;
    const string cards = (hand + " " + board).c_str();

    myServices->logger().verbose("Calling psim for hand equity computing, cards = " + cards +
                                 ", nbOpponents = " + to_string(nbOpponents) +
                                 ", maxOpponentsStrengths = " + to_string(maxOpponentsStrengths));

    myServices->logger().verbose("  --> Calling psim for SimulateHand");

    SimulateHand(cards.c_str(), &r, 0, 1, 5000);

    float win = r.win; // save the value

    myServices->logger().verbose("  --> Calling psim for SimulateHandMulti");
    SimulateHandMulti(cards.c_str(), &r, 1000, 300, nbOpponents);
    r.win = win; // because simulateHandMulti doesn't compute 'win'
    r.winRanged = 0;

    if (maxOpponentsStrengths > 0)
    {
        r.winRanged = 1 - maxOpponentsStrengths;
    }
    if (r.winRanged == 0)
    {
        r.winRanged = r.win / 4;
    }
    HandSimulationStats stats = convertSimResults(r);
    myServices->logger().debug("hand equity is computed");
    return stats;
}
pkt::core::PostFlopAnalysisFlags PsimHandEvaluationEngine::analyzeHand(const std::string& hand,
                                                                       const std::string& board)
{
    ensureServicesInitialized();

    myServices->logger().debug("Calling psim for postflop hand analysis");

    PostFlopState r;
    GetHandState((hand + board).c_str(), &r);
    PostFlopAnalysisFlags flags = convertPostFlopState(r);

    myServices->logger().debug("postflop hand analysis is done");

    return flags;
}

} // namespace pkt::infra