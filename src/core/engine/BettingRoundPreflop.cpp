// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BettingRoundPreflop.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "core/interfaces/IHand.h"
#include "core/services/GlobalServices.h"

#include "Player.h"

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRoundPreflop::BettingRoundPreflop(const GameEvents& events, IHand* hi, unsigned dP, int sB)
    : BettingRound(events, hi, dP, sB, GameStatePreflop)
{
    setHighestSet(2 * getSmallBlind());
}

BettingRoundPreflop::~BettingRoundPreflop() = default;

void BettingRoundPreflop::run()
{
    if (getFirstRun())
    {
        GlobalServices::instance().logger()->info(
            "\n\n************************* PREFLOP *************************\n\n");
        handleFirstRunPreflop();
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

void BettingRoundPreflop::handleFirstRunPreflop()
{
    GlobalServices::instance().logger()->verbose("Handling first run of preflop.");

    PlayerListIterator bigBlindPositionIt = getHand()->getRunningPlayerFromId(getBigBlindPositionId());

    if (getHand()->getSeatsList()->size() > 2)
    {
        handleMultiPlayerFirstRun(bigBlindPositionIt);
    }
    else
    {
        handleHeadsUpFirstRun(bigBlindPositionIt);
    }

    setCurrentPlayersTurnId(getFirstRoundLastPlayersTurnId());
    setFirstRun(false);

    GlobalServices::instance().logger()->verbose("First run of preflop completed. Current player's turn ID: " +
                                                 std::to_string(getCurrentPlayersTurnId()));
}

void BettingRoundPreflop::handleMultiPlayerFirstRun(PlayerListIterator bigBlindPositionIt)
{
    GlobalServices::instance().logger()->verbose("Handling first run for multi-player preflop.");

    if (bigBlindPositionIt == getHand()->getRunningPlayersList()->end())
    {
        PlayerListIterator smallBlindPositionIt = getHand()->getRunningPlayerFromId(getSmallBlindPositionId());

        if (smallBlindPositionIt == getHand()->getRunningPlayersList()->end())
        {
            findLastActivePlayerBeforeSmallBlind();
        }
        else
        {
            setFirstRoundLastPlayersTurnId(getSmallBlindPositionId());
            GlobalServices::instance().logger()->verbose("Small blind is the last player before the first action.");
        }
    }
    else
    {
        setFirstRoundLastPlayersTurnId(getBigBlindPositionId());
        GlobalServices::instance().logger()->verbose("Big blind is the last player before the first action.");
    }
}

void BettingRoundPreflop::handleHeadsUpFirstRun(PlayerListIterator bigBlindPositionIt)
{
    GlobalServices::instance().logger()->verbose("Handling first run for heads-up preflop.");

    if (bigBlindPositionIt == getHand()->getRunningPlayersList()->end())
    {
        PlayerListIterator smallBlindPositionIt = getHand()->getRunningPlayerFromId(getSmallBlindPositionId());

        if (smallBlindPositionIt == getHand()->getRunningPlayersList()->end())
        {
            GlobalServices::instance().logger()->verbose("No running players found. Heads-up showdown.");
        }
        else
        {
            setFirstRoundLastPlayersTurnId(getSmallBlindPositionId());
            GlobalServices::instance().logger()->verbose("Small blind is the last player before the first action.");
        }
    }
    else
    {
        setFirstRoundLastPlayersTurnId(getBigBlindPositionId());
        GlobalServices::instance().logger()->verbose("Big blind is the last player before the first action.");
    }
}

void BettingRoundPreflop::findLastActivePlayerBeforeSmallBlind()
{
    GlobalServices::instance().logger()->verbose("Finding the last active player before the small blind.");

    PlayerListIterator it = getHand()->getPlayerSeatFromId(getSmallBlindPositionId());
    if (it == getHand()->getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::ActivePlayerNotFound);
    }

    if (it == getHand()->getSeatsList()->begin())
    {
        it = getHand()->getSeatsList()->end();
    }
    --it;

    setFirstRoundLastPlayersTurnId((*it)->getId());
    GlobalServices::instance().logger()->verbose("Last active player before small blind found. Player ID: " +
                                                 std::to_string(getFirstRoundLastPlayersTurnId()));
}

bool BettingRoundPreflop::checkAllHighestSet()
{
    GlobalServices::instance().logger()->verbose("Checking if all running players have the highest set.");

    bool allHighestSet = true;
    for (PlayerListConstIterator itC = getHand()->getRunningPlayersList()->begin();
         itC != getHand()->getRunningPlayersList()->end(); ++itC)
    {
        if (getHighestSet() != (*itC)->getSet())
        {
            allHighestSet = false;
            break;
        }
    }

    GlobalServices::instance().logger()->verbose("All highest set check result: " + std::to_string(allHighestSet));
    return allHighestSet;
}

void BettingRoundPreflop::proceedToFlop()
{
    GlobalServices::instance().logger()->verbose("Proceeding to the flop.");

    getHand()->setCurrentRoundState(GameStateFlop);

    for (PlayerListConstIterator itC = getHand()->getRunningPlayersList()->begin();
         itC != getHand()->getRunningPlayersList()->end(); ++itC)
    {
        (*itC)->setAction(PlayerActionNone);
    }

    getHand()->getBoard()->collectSets();
    getHand()->getBoard()->collectPot();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(getHand()->getBoard()->getPot());
    }

    if (myEvents.onRefreshSet)
    {
        myEvents.onRefreshSet();
    }

    if (myEvents.onRefreshCash)
    {
        myEvents.onRefreshCash();
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        if (myEvents.onRefreshAction)
        {
            myEvents.onRefreshAction(i, PlayerActionNone);
        }
    }

    getHand()->resolveHandConditions();
    GlobalServices::instance().logger()->verbose("Flop setup completed.");
}

void BettingRoundPreflop::handleNextPlayerTurn()
{
    GlobalServices::instance().logger()->verbose("Determining the next player's turn.");

    PlayerListConstIterator currentPlayersTurnIt = getHand()->getRunningPlayerFromId(getCurrentPlayersTurnId());
    if (currentPlayersTurnIt == getHand()->getRunningPlayersList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }

    ++currentPlayersTurnIt;
    if (currentPlayersTurnIt == getHand()->getRunningPlayersList()->end())
    {
        currentPlayersTurnIt = getHand()->getRunningPlayersList()->begin();
    }

    setCurrentPlayersTurnId((*currentPlayersTurnIt)->getId());

    if (getCurrentPlayersTurnId() == getFirstRoundLastPlayersTurnId())
    {
        setFirstRound(false);
    }

    currentPlayersTurnIt = getHand()->getRunningPlayerFromId(getCurrentPlayersTurnId());
    if (currentPlayersTurnIt == getHand()->getRunningPlayersList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }
    (*currentPlayersTurnIt)->setTurn(true);

    if (myEvents.onRefreshPlayersActiveInactiveStyles)
    {
        myEvents.onRefreshPlayersActiveInactiveStyles(getCurrentPlayersTurnId(), 2);
    }

    if (myEvents.onRefreshAction)
    {
        myEvents.onRefreshAction(getCurrentPlayersTurnId(), PlayerActionNone);
    }

    if ((*currentPlayersTurnIt)->getName() == HumanPlayer::getName())
    {
        if (myEvents.onDoHumanAction)
        {
            myEvents.onDoHumanAction();
        }
    }
    else
    {
        if (myEvents.onBettingRoundAnimation)
        {
            myEvents.onBettingRoundAnimation(getBettingRoundId());
        }
    }

    GlobalServices::instance().logger()->verbose("Next player's turn determined. Player ID: " +
                                                 std::to_string(getCurrentPlayersTurnId()));
}
} // namespace pkt::core
