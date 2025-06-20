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

BettingRoundPreflop::~BettingRoundPreflop()
{
}

void BettingRoundPreflop::run()
{

    if (getFirstRun())
    {
        GlobalServices::instance().logger()->info(
            "\n\n************************* PREFLOP *************************\n\n");
        PlayerListIterator it;

        // search bigBlindPosition in runningPlayerList
        PlayerListIterator bigBlindPositionIt = getHand()->getRunningPlayerIt(getBigBlindPositionId());

        // more than 2 players are still active -> runningPlayerList is not empty
        if (getHand()->getActivePlayerList()->size() > 2)
        {

            // bigBlindPlayer not found in runningPlayerList (he is all in) -> bigBlindPlayer is not the running player
            // before first action player
            if (bigBlindPositionIt == getHand()->getRunningPlayerList()->end())
            {

                // search smallBlindPosition in runningPlayerList
                PlayerListIterator smallBlindPositionIt = getHand()->getRunningPlayerIt(getSmallBlindPositionId());

                // smallBlindPlayer not found in runningPlayerList (he is all in) -> next active player before
                // smallBlindPlayer is running player before first action player
                if (smallBlindPositionIt == getHand()->getRunningPlayerList()->end())
                {

                    it = getHand()->getActivePlayerIt(getSmallBlindPositionId());
                    if (it == getHand()->getActivePlayerList()->end())
                    {
                        throw Exception(__FILE__, __LINE__, EngineError::ACTIVE_PLAYER_NOT_FOUND);
                    }

                    if (it == getHand()->getActivePlayerList()->begin())
                        it = getHand()->getActivePlayerList()->end();
                    --it;

                    setFirstRoundLastPlayersTurnId((*it)->getID());
                }
                // smallBlindPlayer found in runningPlayerList -> running player before first action player
                else
                {
                    setFirstRoundLastPlayersTurnId(getSmallBlindPositionId());
                }
            }
            // bigBlindPlayer found in runningPlayerList -> player before first action player
            else
            {
                setFirstRoundLastPlayersTurnId(getBigBlindPositionId());
            }
        }
        // heads up -> dealer/smallBlindPlayer is first action player and bigBlindPlayer is player before
        else
        {

            // bigBlindPlayer not found in runningPlayerList (he is all in) -> only smallBlind has to choose fold or
            // call the bigBlindAmount
            if (bigBlindPositionIt == getHand()->getRunningPlayerList()->end())
            {

                // search smallBlindPosition in runningPlayerList
                PlayerListIterator smallBlindPositionIt = getHand()->getRunningPlayerIt(getSmallBlindPositionId());

                // smallBlindPlayer not found in runningPlayerList (he is all in) -> no running player -> showdown and
                // no firstRoundLastPlayersTurnId is used
                if (smallBlindPositionIt == getHand()->getRunningPlayerList()->end())
                {
                }
                // smallBlindPlayer found in runningPlayerList -> running player before first action player (himself)
                else
                {
                    setFirstRoundLastPlayersTurnId(getSmallBlindPositionId());
                }
            }
            else
            {
                setFirstRoundLastPlayersTurnId(getBigBlindPositionId());
            }
        }

        setCurrentPlayersTurnId(getFirstRoundLastPlayersTurnId());

        setFirstRun(false);
    }

    bool allHighestSet = true;
    PlayerListConstIterator it_c;

    // check if all running players have same sets (else allHighestSet = false)
    for (it_c = getHand()->getRunningPlayerList()->begin(); it_c != getHand()->getRunningPlayerList()->end(); ++it_c)
    {
        if (getHighestSet() != (*it_c)->getSet())
        {
            allHighestSet = false;
            break;
        }
    }

    // determine next player
    PlayerListConstIterator currentPlayersTurnIt = getHand()->getRunningPlayerIt(getCurrentPlayersTurnId());
    if (currentPlayersTurnIt == getHand()->getRunningPlayerList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RUNNING_PLAYER_NOT_FOUND);
    }

    ++currentPlayersTurnIt;
    if (currentPlayersTurnIt == getHand()->getRunningPlayerList()->end())
        currentPlayersTurnIt = getHand()->getRunningPlayerList()->begin();

    setCurrentPlayersTurnId((*currentPlayersTurnIt)->getID());

    // prfen, ob Preflop wirklich dran ist
    if (!getFirstRound() && allHighestSet && getHand()->getRunningPlayerList()->size() != 1)
    {

        // Preflop nicht dran, weil wir nicht mehr in erster PreflopRunde und alle Sets gleich sind
        // also gehe in Flop
        getHand()->setCurrentRound(GameStateFlop);

        // Action loeschen und ActionButtons refresh
        for (it_c = getHand()->getRunningPlayerList()->begin(); it_c != getHand()->getRunningPlayerList()->end();
             ++it_c)
        {
            (*it_c)->setAction(PlayerActionNone);
        }

        // Sets in den Pot verschieben und Sets = 0 und Pot-refresh
        getHand()->getBoard()->collectSets();
        getHand()->getBoard()->collectPot();

        if (myEvents.onPotUpdated)
            myEvents.onPotUpdated(getHand()->getBoard()->getPot());

        if (myEvents.onRefreshSet)
            myEvents.onRefreshSet();

        if (myEvents.onRefreshCash)
            myEvents.onRefreshCash();

        for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
        {
            if (myEvents.onRefreshAction)
                myEvents.onRefreshAction(i, PlayerActionNone);
        }

        getHand()->switchRounds();
    }
    else
    {
        // lastPlayersTurn -> PreflopFirstRound is over
        if (getCurrentPlayersTurnId() == getFirstRoundLastPlayersTurnId())
        {
            setFirstRound(false);
        }

        currentPlayersTurnIt = getHand()->getRunningPlayerIt(getCurrentPlayersTurnId());
        if (currentPlayersTurnIt == getHand()->getRunningPlayerList()->end())
        {
            throw Exception(__FILE__, __LINE__, EngineError::RUNNING_PLAYER_NOT_FOUND);
        }
        (*currentPlayersTurnIt)->setTurn(true);

        // highlight active players groupbox and clear action
        if (myEvents.onRefreshPlayersActiveInactiveStyles)
            myEvents.onRefreshPlayersActiveInactiveStyles(getCurrentPlayersTurnId(), 2);

        if (myEvents.onRefreshAction)
            myEvents.onRefreshAction(getCurrentPlayersTurnId(), PlayerActionNone);

        if (getCurrentPlayersTurnId() == 0)
        {
            if (myEvents.onDoHumanAction)
                myEvents.onDoHumanAction();
        }
        else
        {
            if (myEvents.onBettingRoundAnimation)
                myEvents.onBettingRoundAnimation(getBettingRoundID());
        }
    }
}
} // namespace pkt::core
