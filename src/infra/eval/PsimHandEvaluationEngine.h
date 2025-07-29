#pragma once
#include <string>
#include <third_party/psim/psim.hpp>
#include "core/interfaces/IHandEvaluationEngine.h"

namespace pkt::infra
{

/// Poker hand evaluator using the psim library
class PsimHandEvaluationEngine : public pkt::core::IHandEvaluationEngine
{
  public:
    unsigned int rankHand(const char* hand) override;
    const char* getEngineName() const override { return "psim"; }
    pkt::core::HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                      const int nbOpponents, float maxOpponentsStrengths = 0) override;
    pkt::core::PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) override;

  private:
    pkt::core::HandSimulationStats convertSimResults(const SimResults& s);
    pkt::core::PostFlopAnalysisFlags convertPostFlopState(const PostFlopState& src);
};

} // namespace pkt::infra