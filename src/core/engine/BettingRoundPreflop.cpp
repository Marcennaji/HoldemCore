/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "BettingRoundPreflop.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "core/interfaces/IHand.h"

#include "Player.h"

namespace pkt::core
{

using namespace std;

BettingRoundPreflop::BettingRoundPreflop(GameEvents* events, ILogger* logger, IHand* hi, unsigned dP, int sB)
    : BettingRound(events, logger, hi, dP, sB, GAME_STATE_PREFLOP)
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
#ifdef LOG_POKER_EXEC
        cout << endl << endl << "************************* PREFLOP *************************" << endl << endl;
#endif
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
        getHand()->setCurrentRound(GAME_STATE_FLOP);

        // Action loeschen und ActionButtons refresh
        for (it_c = getHand()->getRunningPlayerList()->begin(); it_c != getHand()->getRunningPlayerList()->end();
             ++it_c)
        {
            (*it_c)->setAction(PLAYER_ACTION_NONE);
        }

        // Sets in den Pot verschieben und Sets = 0 und Pot-refresh
        getHand()->getBoard()->collectSets();
        getHand()->getBoard()->collectPot();

        if (myEvents && myEvents->onPotUpdated)
            myEvents->onPotUpdated(getHand()->getBoard()->getPot());

        if (myEvents && myEvents->onRefreshSet)
            myEvents->onRefreshSet();

        if (myEvents && myEvents->onRefreshCash)
            myEvents->onRefreshCash();

        for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
        {
            if (myEvents && myEvents->onRefreshAction)
                myEvents->onRefreshAction(i, PLAYER_ACTION_NONE);
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
        if (myEvents && myEvents->onRefreshPlayersActiveInactiveStyles)
            myEvents->onRefreshPlayersActiveInactiveStyles(getCurrentPlayersTurnId(), 2);

        if (myEvents && myEvents->onRefreshAction)
            myEvents->onRefreshAction(getCurrentPlayersTurnId(), PLAYER_ACTION_NONE);

        if (getCurrentPlayersTurnId() == 0)
        {
            if (myEvents && myEvents->onDoHumanAction)
                myEvents->onDoHumanAction();
        }
        else
        {
            if (myEvents && myEvents->onBettingRoundAnimation)
                myEvents->onBettingRoundAnimation(getBettingRoundID());
        }
    }
}
} // namespace pkt::core
