#pragma once
#include <memory>
#include <string>
#include "core/interfaces/HandEvaluationEngine.h"
#include "core/interfaces/Logger.h"

struct SimResults;
struct PostFlopState;

namespace pkt::infra
{

/// Poker hand evaluator using the psim library
class PsimHandEvaluationEngine : public pkt::core::HandEvaluationEngine
{
  public:
    PsimHandEvaluationEngine(std::shared_ptr<pkt::core::Logger> logger);
    PsimHandEvaluationEngine();

    unsigned int rankHand(const char* hand) override;
    const char* getEngineName() const override { return "psim"; }
    pkt::core::HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                      const int nbOpponents, float maxOpponentsStrengths) override;
    pkt::core::PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) override;

  private:
    pkt::core::HandSimulationStats convertSimResults(const SimResults& s);
    pkt::core::PostFlopAnalysisFlags convertPostFlopState(const PostFlopState& src);
    
    pkt::core::Logger& getLogger() const;

    std::shared_ptr<pkt::core::Logger> m_logger;
};

} // namespace pkt::infra