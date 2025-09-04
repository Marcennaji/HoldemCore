#pragma once
#include <string>
#include "core/interfaces/IHandEvaluationEngine.h"

struct SimResults;
struct PostFlopState;

namespace pkt::core
{

/// Poker hand evaluator using the psim library
class NullHandEvaluationEngine : public pkt::core::IHandEvaluationEngine
{
  public:
    unsigned int rankHand(const char* hand) override { return 0; }
    const char* getEngineName() const override { return "NullHandEvaluationEngine"; }
    pkt::core::HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                      const int nbOpponents, float maxOpponentsStrengths) override
    {
        pkt::core::HandSimulationStats stats;
        return stats;
    }
    pkt::core::PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) override
    {
        pkt::core::PostFlopAnalysisFlags flags;
        return flags;
    }
};

} // namespace pkt::core
