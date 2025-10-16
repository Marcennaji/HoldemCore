#include "PsimHandEvaluationEngine.h"
#include "adapters/infrastructure/logger/NullLogger.h"
#include "core/ports/HandEvaluationEngine.h"
#include "third_party/psim/psim.hpp"

namespace pkt::infra
{

using namespace pkt::core;
using namespace std;

PsimHandEvaluationEngine::PsimHandEvaluationEngine(std::shared_ptr<pkt::core::Logger> logger) : m_logger(logger)
{
}

PsimHandEvaluationEngine::PsimHandEvaluationEngine()
{
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
    SimResults r;
    const string cards = (hand + " " + board).c_str();

    if (m_logger)
    {
        m_logger->verbose("Calling psim for hand equity computing, cards = " + cards + ", nbOpponents = " +
                          to_string(nbOpponents) + ", maxOpponentsStrengths = " + to_string(maxOpponentsStrengths));
    }

    if (m_logger)
    {
        m_logger->verbose("  --> Calling psim for SimulateHand");
    }

    SimulateHand(cards.c_str(), &r, 0, 1, 5000);

    float win = r.win; // save the value

    if (m_logger)
    {
        m_logger->verbose("  --> Calling psim for SimulateHandMulti");
    }
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
    if (m_logger)
    {
        m_logger->debug("hand equity is computed");
    }
    return stats;
}
pkt::core::PostFlopAnalysisFlags PsimHandEvaluationEngine::analyzeHand(const std::string& hand,
                                                                       const std::string& board)
{
    if (m_logger)
    {
        m_logger->debug("Calling psim for postflop hand analysis");
    }

    PostFlopState r;
    // Build full hand string with space between hole cards and board for consistency
    std::string fullHand = hand;
    if (!board.empty())
    {
        fullHand += " " + board;
    }
    GetHandState(fullHand.c_str(), &r);
    PostFlopAnalysisFlags flags = convertPostFlopState(r);

    if (m_logger)
    {
        m_logger->debug("postflop hand analysis is done");
    }

    return flags;
}

} // namespace pkt::infra