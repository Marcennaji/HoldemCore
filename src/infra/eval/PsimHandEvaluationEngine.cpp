#include "PsimHandEvaluationEngine.h"
#include <third_party/psim/psim.hpp>

namespace pkt::infra
{

using namespace pkt::core;
using namespace std;

unsigned int PsimHandEvaluationEngine::rankHand(const char* hand)
{
    return ::rankHand(hand);
}
HandSimulationStats PsimHandEvaluationEngine::convertSimResults(const SimResults& s)
{
    return {s.win,    s.tie,  s.lose, s.winRanged, s.winSd,      s.tieSd,
            s.loseSd, s.dNow, s.d94,  s.d90,       s.evaluations

    };
}
PostFlopAnalysisFlags PsimHandEvaluationEngine::convertPostFlopState(const PostFlopState& src)
{

    return {
        src.IsNoPair,   src.IsOnePair, src.IsPocketPair, src.IsTwoPair, src.IsTrips,
        src.IsStraight, src.IsFlush,   src.IsFullHouse,  src.IsQuads,   src.IsStFlush,
    };
}
HandSimulationStats PsimHandEvaluationEngine::simulateHandEquity(const std::string& hand, const std::string& board,
                                                                 const int nbOpponents, float maxOpponentsStrengths)
{
    SimResults r;
    const string cards = (hand + board).c_str();

    simulateHand(cards.c_str(), &r, 0, 1, 0);

    float win = r.win; // save the value

    simulateHandMulti(cards.c_str(), &r, 200, 100, nbOpponents);
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
    return stats;
}
pkt::core::PostFlopAnalysisFlags PsimHandEvaluationEngine::analyzeHand(const std::string& hand,
                                                                       const std::string& board)
{
    PostFlopState r;

    getHandState((hand + board).c_str(), &r);

    PostFlopAnalysisFlags flags = convertPostFlopState(r);

    return flags;
}

} // namespace pkt::infra