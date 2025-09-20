#pragma once
#include <memory>
#include <string>
#include "core/interfaces/HandEvaluationEngine.h"

struct SimResults;
struct PostFlopState;

namespace pkt::core
{
class ServiceContainer;
}

namespace pkt::infra
{

/// Poker hand evaluator using the psim library
class PsimHandEvaluationEngine : public pkt::core::HandEvaluationEngine
{
  public:
    /// Default constructor using AppServiceContainer
    PsimHandEvaluationEngine();

    /// Constructor with ServiceContainer dependency injection
    explicit PsimHandEvaluationEngine(std::shared_ptr<pkt::core::ServiceContainer> services);

    unsigned int rankHand(const char* hand) override;
    const char* getEngineName() const override { return "psim"; }
    pkt::core::HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                      const int nbOpponents, float maxOpponentsStrengths) override;
    pkt::core::PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) override;

  private:
    pkt::core::HandSimulationStats convertSimResults(const SimResults& s);
    pkt::core::PostFlopAnalysisFlags convertPostFlopState(const PostFlopState& src);

    /// Ensures services are initialized (lazy initialization)
    void ensureServicesInitialized();

    /// ServiceContainer for dependency injection
    std::shared_ptr<pkt::core::ServiceContainer> myServices;
};

} // namespace pkt::infra