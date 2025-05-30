
#pragma once

#include <core/player/Player.h>
#include <third_party/psim/psim.hpp>

namespace pkt::core
{

struct CurrentHandContext
{
    // data that is common to all players
    GameState gameState;
    int preflopRaisesNumber;
    int preflopCallsNumber;
    int flopBetsOrRaisesNumber; // including allins
    int turnBetsOrRaisesNumber;
    int riverBetsOrRaisesNumber;
    int nbPlayers;        // total number of players in the current hand, including the ones who folded
    int nbRunningPlayers; // running players = players who raised + players who called + players who didn't act
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

} // namespace pkt::core