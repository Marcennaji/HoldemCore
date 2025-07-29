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
    bool usesFirst = false;
    bool usesSecond = false;

    /// What is our actual hand rank?
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

    bool isTopPair = false; ///< We paired the highest card one the board with one in our hand.
    bool isMiddlePair =
        false; ///< We paired the board, but it's not top or bottom pair. (may be 2nd best, 3rd best, etc.)
    bool isBottomPair = false; ///< We paired the lowest card one the board with one in our hand.
    bool isOverPair = false;   ///< We have a pocket pair, and it's bigger than any card on the board.
    bool isOverCards = false;  ///< We just have two nonpaired cards bigger than anything on the board.

    bool isStraightDrawPossible = false; ///< Could someone have 4 to a straight?
    bool isStraightPossible = false;     /// Could someone have a straight?

    bool isFlushDrawPossible = false; ///< Could someone have 4 to a suit?
    bool isFlushPossible = false;     ///< Could someone have 5 to a suit?

    bool isFullHousePossible = false; ///< is a fullhouse or quads possible?

    bool is3Flush = false; ///< Do we have exactly 3 cards to a flush?
    bool is4Flush = false; ///< Do we have exactly 4 cards to a flush?

    unsigned int flushOuts : 4;    ///< How many cards will give us flush?
    unsigned int straightOuts : 4; ///< How many cards will give us straight?
    unsigned int betterOuts : 4;   ///< How many cards will give us a fullhouse or better?
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