#pragma once

namespace pkt::core
{

/// Abstract interface for poker hand evaluation engines
class IHandEvaluationEngine
{
  public:
    virtual ~IHandEvaluationEngine() = default;

    /// Evaluates a poker hand from a string representation
    /// \param hand String representation of the hand (e.g., "Ah Kd Qs Jc Ts 9h 8s")
    /// \return Hand strength score where higher means better
    virtual unsigned int rankHand(const char* hand) = 0;

    /// Gets the name of the evaluation engine
    virtual const char* getEngineName() const = 0;
};

} // namespace pkt::core