
#pragma once

#include <core/player/Player.h>

namespace pkt::core
{

struct CurrentHandContext
{
    // data that is common to all players
    const GameState gameState;
    const int preflopRaisesNumber;
    const int preflopCallsNumber;
    const int flopRaisesNumber;
    const int flopCallsNumber;
    const int flopBetsOrRaisesNumber; // including allins
    const int turnBetsOrRaisesNumber;
    const int riverBetsOrRaisesNumber;
    const int nbPlayers;        // total number of players in the current hand, including the ones who folded
    const int nbRunningPlayers; // running players = players who raised + players who called + players who didn't act
                                // yet. All-in players are not "running" any more
    std::shared_ptr<Player> preflopLastRaiser;
    std::shared_ptr<Player> flopLastRaiser;
    std::shared_ptr<Player> turnLastRaiser;
    std::shared_ptr<Player> lastVPIPPlayer; // last player who voluntarily put money in the pot
    std::vector<PlayerPosition> callersPositions;
    int pot;
    int potOdd;
    int sets;
    int highestSet;
    bool isPreflopBigBet;
    int smallBlind;
    std::string stringBoard; // string representation of the board, e.g. "2H 3D 4C 5S 6H"

    // data that depends on the player that is using the strategy
    float myPreflopCallingRange;
    int myCash;
    int mySet;
    int myM;
    int myID;
    std::string myCard1;
    std::string myCard2;
    PlayerPosition myPosition;
    CurrentHandActions myCurrentHandActions;
    bool myCanBluff;     // true if the player can bluff in the current betting round
    bool myHavePosition; // true if the player is last to act in the current betting round
    bool myPreflopIsAggressor;
    bool myFlopIsAggressor;
    bool myTurnIsAggressor;
    bool myRiverIsAggressor;
    PostFlopState myPostFlopState;
    SimResults myHandSimulation;
};

class IBotStrategy
{
  public:
    virtual bool preflopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool flopShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool turnShouldCall(CurrentHandContext&, bool deterministic = false) = 0;
    virtual bool riverShouldCall(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int preflopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int flopShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldRaise(CurrentHandContext&, bool deterministic = false) = 0;

    virtual int flopShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int turnShouldBet(CurrentHandContext&, bool deterministic = false) = 0;
    virtual int riverShouldBet(CurrentHandContext&, bool deterministic = false) = 0;

    virtual ~IBotStrategy() = default;

  protected:
    void initializeRanges(const int utgHeadsUpRange, const int utgFullTableRange);
    float getPreflopCallingRange(CurrentHandContext&, bool deterministic = false) const;
    float getPreflopRaisingRange(CurrentHandContext&, bool deterministic = false) const;
    int getRange(PlayerPosition p, const int nbPlayers) const;
    int computePreflopRaiseAmount(CurrentHandContext&, bool deterministic = false);

    bool myShouldCall = false;
    bool myShouldRaise = false;

  private:
    // vector index is player position, value is range %
    std::vector<int> UTG_STARTING_RANGE;
    std::vector<int> UTG_PLUS_ONE_STARTING_RANGE;
    std::vector<int> UTG_PLUS_TWO_STARTING_RANGE;
    std::vector<int> MIDDLE_STARTING_RANGE;
    std::vector<int> MIDDLE_PLUS_ONE_STARTING_RANGE;
    std::vector<int> LATE_STARTING_RANGE;
    std::vector<int> CUTOFF_STARTING_RANGE;
    std::vector<int> BUTTON_STARTING_RANGE;
    std::vector<int> SB_STARTING_RANGE;
    std::vector<int> BB_STARTING_RANGE;
};
} // namespace pkt::core
