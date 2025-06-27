#pragma once
#include "core/interfaces/IHandEvaluationEngine.h"

namespace pkt::infra
{

/// Poker hand evaluator using the psim library
class PsimHandEvaluationEngine : public pkt::core::IHandEvaluationEngine
{
  public:
    unsigned int rankHand(const char* hand) override;
    const char* getEngineName() const override { return "psim"; }
};

} // namespace pkt::infra