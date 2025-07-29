
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/Player.h>
#include "core/interfaces/IHandEvaluationEngine.h"

namespace pkt::core::player
{

struct CurrentHandContext
{
    void reset(); // all fields are zeroed or nullptr

    // data that is common to all players
    GameState gameState = GameStateNone; // current game state, e.g. PREFLOP, FLOP, TURN, RIVER, SHOWDOWN
    int nbChecks = 0;                    // number of checks in the current betting round
    int nbRaises = 0;                    // number of raises in the current betting round
    int nbBets = 0;                      // number of bets in the current betting round
    int nbAllins = 0;                    // number of all-ins in the current betting round
    int nbCalls = 0;                     // number of calls in the current betting round
    int preflopRaisesNumber = 0;
    int preflopCallsNumber = 0;
    int flopBetsOrRaisesNumber = 0; // including allins
    int turnBetsOrRaisesNumber = 0;
    int riverBetsOrRaisesNumber = 0;
    int nbPlayers = 0;        // total number of players in the current hand, including the ones who folded
    int nbRunningPlayers = 0; // running players = players who raised + players who called + players who didn't act
                              // yet. All-in players are not "running" any more
    std::shared_ptr<Player> preflopLastRaiser = nullptr;
    std::shared_ptr<Player> flopLastRaiser = nullptr;
    std::shared_ptr<Player> turnLastRaiser = nullptr;
    std::shared_ptr<Player> lastVPIPPlayer = nullptr; // last player who voluntarily put money in the pot
    std::vector<PlayerPosition> callersPositions;
    int pot = 0;
    int potOdd = 0;
    int sets = 0;
    int highestSet = 0;
    bool isPreflopBigBet = false;
    int smallBlind = 0;
    std::string stringBoard; // string representation of the board, e.g. "2H 3D 4C 5S 6H"

    // data that depends on the player that is using the strategy
    float myPreflopCallingRange = -1;
    int myCash = 0;
    int mySet = 0;
    int myM = 0;
    int myID = 0;
    std::string myCard1 = "";
    std::string myCard2 = "";
    PlayerPosition myPosition = PlayerPosition::UNKNOWN;
    CurrentHandActions myCurrentHandActions;
    PlayerStatistics myStatistics{};
    bool myCanBluff = false;     // true if the player can bluff in the current betting round
    bool myHavePosition = false; // true if the player is last to act in the current betting round
    bool myPreflopIsAggressor = false;
    bool myFlopIsAggressor = false;
    bool myTurnIsAggressor = false;
    bool myRiverIsAggressor = false;
    bool myIsInVeryLooseMode = false; // true if the player is in very loose mode (e.g. playing a lot of hands)
    PostFlopAnalysisFlags myPostFlopAnalysisFlags{};

    HandSimulationStats myHandSimulation = {
        0.0f, // win
        0.0f, // tie
        0.0f, // lose
        0.0f, // winRanged
        0.0f, // winSd
        0.0f, // tieSd
        0.0f, // loseSd
        0.0f, // dNow
        0.0f, // d94
        0.0f  // d90
    };
};

} // namespace pkt::core::player