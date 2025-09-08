// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "core/engine/deprecated/BettingRoundPreflop.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "core/interfaces/IHand.h"
#include "core/services/GlobalServices.h"

#include "Helpers.h"
#include "core/player/deprecated/Player.h"

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRoundPreflop::BettingRoundPreflop(const GameEvents& events, IHand* hi, unsigned dP, int sB)
    : BettingRound(events, hi, dP, sB, Preflop)
{
    setHighestSet(2 * getSmallBlind());
}

BettingRoundPreflop::~BettingRoundPreflop() = default;

void BettingRoundPreflop::run()
{
    if (getFirstRun())
    {
        GlobalServices::instance().logger().info("\n\n************************* PREFLOP *************************\n\n");
        handleFirstRun();
    }

    if (checkAllHighestSet())
    {
        proceedToFlop();
    }
    else
    {
        handleNextPlayerTurn();
    }
}

void BettingRoundPreflop::handleFirstRun()
{
    GlobalServices::instance().logger().verbose("Handling first run of preflop.");

    auto bigBlindPositionIt = getPlayerListIteratorById(getHand()->getActingPlayersList(), getBigBlindPlayerId());

    if (getHand()->getSeatsList()->size() > 2)
    {
        handleMultiPlayerFirstRun(bigBlindPositionIt);
    }
    else
    {
        handleHeadsUpFirstRun(bigBlindPositionIt);
    }

    setCurrentPlayerTurnId(getFirstRoundLastPlayersTurnId());
    setFirstRun(false);

    GlobalServices::instance().logger().verbose("First run of preflop completed. Current player's turn ID: " +
                                                std::to_string(getCurrentPlayerTurnId()));
}

void BettingRoundPreflop::handleMultiPlayerFirstRun(PlayerListIterator bigBlindPositionIt)
{
    GlobalServices::instance().logger().verbose("Handling first run for multi-player preflop.");

    if (bigBlindPositionIt == getHand()->getActingPlayersList()->end())
    {
        auto smallBlindPositionIt =
            getPlayerListIteratorById(getHand()->getActingPlayersList(), getSmallBlindPlayerId());

        if (smallBlindPositionIt == getHand()->getActingPlayersList()->end())
        {
            findLastActivePlayerBeforeSmallBlind();
        }
        else
        {
            setFirstRoundLastPlayersTurnId(getSmallBlindPlayerId());
            GlobalServices::instance().logger().verbose("Small blind is the last player before the first action.");
        }
    }
    else
    {
        setFirstRoundLastPlayersTurnId(getBigBlindPlayerId());
        GlobalServices::instance().logger().verbose("Big blind is the last player before the first action.");
    }
}

void BettingRoundPreflop::handleHeadsUpFirstRun(PlayerListIterator bigBlindPositionIt)
{
    GlobalServices::instance().logger().verbose("Handling first run for heads-up preflop.");

    if (bigBlindPositionIt == getHand()->getActingPlayersList()->end())
    {
        auto smallBlindPositionIt =
            getPlayerListIteratorById(getHand()->getActingPlayersList(), getSmallBlindPlayerId());

        if (smallBlindPositionIt == getHand()->getActingPlayersList()->end())
        {
            GlobalServices::instance().logger().verbose("No acting players found. Heads-up showdown.");
        }
        else
        {
            setFirstRoundLastPlayersTurnId(getSmallBlindPlayerId());
            GlobalServices::instance().logger().verbose("Small blind is the last player before the first action.");
        }
    }
    else
    {
        setFirstRoundLastPlayersTurnId(getBigBlindPlayerId());
        GlobalServices::instance().logger().verbose("Big blind is the last player before the first action.");
    }
}

void BettingRoundPreflop::proceedToFlop()
{
    GlobalServices::instance().logger().verbose("Proceeding to the flop.");

    getHand()->setCurrentRoundState(Flop);

    for (auto player = getHand()->getActingPlayersList()->begin(); player != getHand()->getActingPlayersList()->end();
         ++player)
    {
        (*player)->setLastAction({(*player)->getId(), ActionType::None});
    }

    getHand()->getBoard()->collectSets();
    getHand()->getBoard()->collectPot();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(getHand()->getBoard()->getPot());
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        if (myEvents.onPlayerActed)
        {
            myEvents.onPlayerActed({i, ActionType::None, 0});
        }
    }

    getHand()->resolveHandConditions();
    GlobalServices::instance().logger().verbose("Flop setup completed.");
}

void BettingRoundPreflop::handlePlayerTurnEvents(PlayerListIterator it)
{
    if (myEvents.onPlayerStatusChanged)
        myEvents.onPlayerStatusChanged(getCurrentPlayerTurnId(), true);

    if (myEvents.onPlayerActed)
        myEvents.onPlayerActed({getCurrentPlayerTurnId(), ActionType::None, 0});

    if (!it->get()->isBot())
    {
        if (myEvents.onAwaitingHumanInput)
            myEvents.onAwaitingHumanInput();
    }
    else
    {
        GlobalServices::instance().logger().verbose("Giving action to next bot player: " + it->get()->getName());
        giveActionToNextBotPlayer();
    }
}
void BettingRoundPreflop::handleNextPlayerTurn()
{
    GlobalServices::instance().logger().verbose("Preflop: Determining the next player's turn.");

    auto seats = getHand()->getSeatsList();
    auto currentPlayersTurnIt = findPlayerOrThrow(seats, getCurrentPlayerTurnId());

    currentPlayersTurnIt = nextActivePlayer(seats, currentPlayersTurnIt);
    setCurrentPlayerTurnId((*currentPlayersTurnIt)->getId());

    if (getCurrentPlayerTurnId() == getFirstRoundLastPlayersTurnId())
        setFirstRound(false);

    currentPlayersTurnIt = findPlayerOrThrow(seats, getCurrentPlayerTurnId());
    handlePlayerTurnEvents(currentPlayersTurnIt);

    GlobalServices::instance().logger().verbose("Next player's turn determined. Player ID: " +
                                                std::to_string(getCurrentPlayerTurnId()));
}
} // namespace pkt::core
