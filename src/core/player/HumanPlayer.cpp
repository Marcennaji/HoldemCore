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
#include "HumanPlayer.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core
{

using namespace std;

HumanPlayer::HumanPlayer(GameEvents* events, IHandAuditStore* ha, IPlayersStatisticsStore* ps, int id, PlayerType type,
                         std::string name, int sC, bool aS, bool sotS, int mB)
    : Player(events, ha, ps, id, type, name, sC, aS, sotS, mB)
{
}

HumanPlayer::~HumanPlayer()
{
}
void HumanPlayer::action()
{

    switch (currentHand->getCurrentRound())
    {

    case GAME_STATE_PREFLOP:
        // doPreflopAction();
        break;

    case GAME_STATE_FLOP:
        // doFlopAction();
        break;

    case GAME_STATE_TURN:
        // doTurnAction();
        break;

    case GAME_STATE_RIVER:
        // doRiverAction();
        break;

    default:
        break;
    }

    // evaluateBetAmount(); // original bet amount may be modified
    currentHand->getBoard()->collectSets();
    if (myEvents && myEvents->onPotUpdated)
        myEvents->onPotUpdated(currentHand->getBoard()->getPot());

    myTurn = 0;

#ifdef LOG_POKER_EXEC
    cout << endl;
    if (myAction == PLAYER_ACTION_FOLD)
        cout << "FOLD";
    else if (myAction == PLAYER_ACTION_BET)
        cout << "BET " << myBetAmount;
    else if (myAction == PLAYER_ACTION_RAISE)
        cout << "RAISE " << myRaiseAmount;
    else if (myAction == PLAYER_ACTION_CALL)
        cout << "CALL ";
    else if (myAction == PLAYER_ACTION_CHECK)
        cout << "CHECK";
    else if (myAction == PLAYER_ACTION_ALLIN)
        cout << "ALLIN ";
    else if (myAction == PLAYER_ACTION_NONE)
        cout << "NONE";
    else
        cout << "undefined ?";

    cout << endl << "---------------------------------------------------------------------------------" << endl << endl;
#endif

    currentHand->setPreviousPlayerID(myID);

    if (myEvents && myEvents->onActivePlayerActionDone)
        myEvents->onActivePlayerActionDone();

    // currentHand->getGuiInterface()->showCards(myID);//test
}
} // namespace pkt::core
