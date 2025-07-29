#pragma once
#include <string>
#include <vector>

namespace pkt::core
{

struct HandSimulationStats
{
    float win;
    float tie;
    float lose;
    float winRanged;
    float winSd;
    float tieSd;
    float loseSd;
    float dNow;
    float d94;
    float d90;
    int evaluations;
};

struct PostFlopAnalysisFlags
{
    bool isNoPair = false;
    bool isOnePair = false;
    bool isPocketPair = false;
    bool isTwoPair = false;
    bool isTrips = false;
    bool isStraight = false;
    bool isFlush = false;
    bool isFullHouse = false;
    bool isQuads = false;
    bool isStFlush = false;
};

/// Abstract interface for poker hand evaluation engines
class IHandEvaluationEngine
{
  public:
    virtual ~IHandEvaluationEngine() = default;

    /// Evaluates a poker hand from a string representation
    /// \param hand String representation of the hand (e.g., "Ah Kd Qs Jc Ts 9h 8s")
    /// \return Hand strength score where higher means better
    virtual unsigned int rankHand(const char* hand) = 0;

    virtual HandSimulationStats simulateHandEquity(const std::string& hand, const std::string& board,
                                                   const int nbOpponents, float maxOpponentsStrengths) = 0;
    virtual PostFlopAnalysisFlags analyzeHand(const std::string& hand, const std::string& board) = 0;

    /// Gets the name of the evaluation engine
    virtual const char* getEngineName() const = 0;
};

} // namespace pkt::core