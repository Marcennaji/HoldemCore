
// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/deprecated/Player.h>
#include "core/engine/model/PlayerPosition.h"
#include "core/interfaces/IHandEvaluationEngine.h"

namespace pkt::core::player
{
struct PlayersInHandContext
{
    int nbPlayers = 0; // total number of players in the current hand, including the ones who folded
    PlayerList runningPlayersList;
    std::shared_ptr<Player> preflopLastRaiser = nullptr;
    std::shared_ptr<Player> flopLastRaiser = nullptr;
    std::shared_ptr<Player> turnLastRaiser = nullptr;
    std::shared_ptr<Player> lastVPIPPlayer = nullptr; // last player who voluntarily put money in the

    PlayerFsmList runningPlayersListFsm; // list of players still acting in the hand (not folded and not allin)
    std::shared_ptr<PlayerFsm> preflopLastRaiserFsm = nullptr;
    std::shared_ptr<PlayerFsm> flopLastRaiserFsm = nullptr;
    std::shared_ptr<PlayerFsm> turnLastRaiserFsm = nullptr;
    std::shared_ptr<PlayerFsm> lastVPIPPlayerFsm = nullptr; // last player who voluntarily put money in the pot

    std::vector<pkt::core::PlayerPosition> callersPositions;
    std::vector<pkt::core::PlayerPosition> raisersPositions;
};

struct HandBettingContext
{
    int preflopRaisesNumber = 0;
    int preflopCallsNumber = 0;
    int flopBetsOrRaisesNumber = 0; // including allins
    int turnBetsOrRaisesNumber = 0;
    int riverBetsOrRaisesNumber = 0;
    int pot = 0;
    int potOdd = 0;
    int sets = 0;
    int highestBetAmount = 0;
    bool isPreflopBigBet = false;
};

// data that depends on the player that is using the strategy

struct PerPlayerHandActions
{
    bool preflopIsAggressor = false;
    bool flopIsAggressor = false;
    bool turnIsAggressor = false;
    bool riverIsAggressor = false;
    bool isInVeryLooseMode = false; // true if the player is in very loose mode (e.g. playing a lot of hands)
    CurrentHandActions currentHandActions;
};

struct PerPlayerHandContext
{
    PerPlayerHandActions actions;
    bool hasPosition = false; // true if the player is last to act in the current betting round
    float preflopCallingRange = -1;
    int cash = 0;
    int totalBetAmount = 0;
    int m = 0;
    int id = 0;
    std::string card1 = "";
    std::string card2 = "";
    pkt::core::PlayerPosition position = PlayerPosition::UNKNOWN;

    PlayerStatistics statistics{};
    PostFlopAnalysisFlags postFlopAnalysisFlags{}; // what are the possibilities, given the board and the hole cards ?

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

// data that is common to all players
struct HandCommonContext
{
    GameState gameState = None;
    int smallBlind = 0;
    std::string stringBoard; // string representation of the board's cards, for example "As Ks Qs"
    PlayersInHandContext playersContext;
    HandBettingContext bettingContext;
};

struct CurrentHandContext
{
    HandCommonContext commonContext;
    PerPlayerHandContext personalContext;
};

} // namespace pkt::core::player