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

#include "hand.h"
#include "tools.h"
#include "cardsvalue.h"
#include <game_defs.h>
#include <loghelper.h>

#include "exception.h"
#include "engine_msg.h"

#include <iostream>

using namespace std;

Hand::Hand(std::shared_ptr<EngineFactory> f, GuiInterface *g, std::shared_ptr<BoardInterface> b, Log *l, PlayerList sl, PlayerList apl, PlayerList rpl, int id, int sP, unsigned dP, int sB,int sC)
	: myFactory(f), myGui(g),  myBoard(b), myLog(l), seatsList(sl), activePlayerList(apl), runningPlayerList(rpl), myBettingRound(0), myID(id), startQuantityPlayers(sP), dealerPosition(dP), smallBlindPosition(dP), bigBlindPosition(dP), currentRound(GAME_STATE_PREFLOP), roundBeforePostRiver(GAME_STATE_PREFLOP), smallBlind(sB), startCash(sC), previousPlayerID(-1), lastActionPlayerID(0), allInCondition(false),
	  cardsShown(false)
{

	int i, j, k;
	PlayerListIterator it;
#ifdef LOG_POKER_EXEC
	cout << endl << "-------------------------------------------------------------" << endl << endl;
	cout << endl << "HAND " << myID << endl;
#endif

	for(it=seatsList->begin(); it!=seatsList->end(); ++it) {
		(*it)->setHand(this);
		// set myFlipCards 0
		(*it)->setCardsFlip(0, 0);
	}

	// generate cards and assign to board and player
	vector<int> cardsArray = {
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
		26, 27, 28, 29,	30, 31, 32, 33, 34, 35, 36, 37, 38, 
		39,	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,	50, 51
	};
	
	/*
	0 to 12 --> 2d to Ad
	13 to 25 --> 2h to Ah
	26 to 38 --> 2s to As
	39 to 51 --> 2c to Ac
	*/
	
	Tools::ShuffleArrayNonDeterministic(cardsArray);

#ifdef LOG_POKER_EXEC

	static map<string,int> cardStringToInt;

	cardStringToInt["2d"] = 0;
	cardStringToInt["3d"] = 1;
	cardStringToInt["4d"] = 2;
	cardStringToInt["5d"] = 3;
	cardStringToInt["6d"] = 4;
	cardStringToInt["7d"] = 5;
	cardStringToInt["8d"] = 6;
	cardStringToInt["9d"] = 7;
	cardStringToInt["Td"] = 8;
	cardStringToInt["Jd"] = 9;
	cardStringToInt["Qd"] = 10;
	cardStringToInt["Kd"] = 11;
	cardStringToInt["Ad"] = 12;

	cardStringToInt["2h"] = 13;
	cardStringToInt["3h"] = 14;
	cardStringToInt["4h"] = 15;
	cardStringToInt["5h"] = 16;
	cardStringToInt["6h"] = 17;
	cardStringToInt["7h"] = 18;
	cardStringToInt["8h"] = 19;
	cardStringToInt["9h"] = 20;
	cardStringToInt["Th"] = 21;
	cardStringToInt["Jh"] = 22;
	cardStringToInt["Qh"] = 23;
	cardStringToInt["Kh"] = 24;
	cardStringToInt["Ah"] = 25;

	cardStringToInt["2s"] = 26;
	cardStringToInt["3s"] = 27;
	cardStringToInt["4s"] = 28;
	cardStringToInt["5s"] = 29;
	cardStringToInt["6s"] = 30;
	cardStringToInt["7s"] = 31;
	cardStringToInt["8s"] = 32;
	cardStringToInt["9s"] = 33;
	cardStringToInt["Ts"] = 34;
	cardStringToInt["Js"] = 35;
	cardStringToInt["Qs"] = 36;
	cardStringToInt["Ks"] = 37;
	cardStringToInt["As"] = 38;

	cardStringToInt["2c"] = 39;
	cardStringToInt["3c"] = 40;
	cardStringToInt["4c"] = 41;
	cardStringToInt["5c"] = 42;
	cardStringToInt["6c"] = 43;
	cardStringToInt["7c"] = 44;
	cardStringToInt["8c"] = 45;
	cardStringToInt["9c"] = 46;
	cardStringToInt["Tc"] = 47;
	cardStringToInt["Jc"] = 48;
	cardStringToInt["Qc"] = 49;
	cardStringToInt["Kc"] = 50;
	cardStringToInt["Ac"] = 51;


	if (false){ // put 'true', to test a particular board and players cards :

		// board :     
		cardsArray[0] = cardStringToInt["Jd"];
		cardsArray[1] = cardStringToInt["Jc"];
		cardsArray[2] = cardStringToInt["Ts"];
		cardsArray[3] = cardStringToInt["Ks"];
		cardsArray[4] = cardStringToInt["2h"];

		// player 1 (human)
		cardsArray[5] = cardStringToInt["Kc"];
		cardsArray[6] = cardStringToInt["Kh"];

		// player 2
		cardsArray[7] = cardStringToInt["As"];
		cardsArray[8] = cardStringToInt["Qd"];

		// player 3
		cardsArray[9] = cardStringToInt["Ks"];
		cardsArray[10] = cardStringToInt["Kc"];

		// player 4
		cardsArray[11] = cardStringToInt["Ah"];
		cardsArray[12] = cardStringToInt["Ah"];

		// player 5
		cardsArray[13] = cardStringToInt["Qh"];
		cardsArray[14] = cardStringToInt["Qs"];

		// player 6
		cardsArray[15] = cardStringToInt["2c"];
		cardsArray[16] = cardStringToInt["2h"];

		// player 7
		cardsArray[17] = cardStringToInt["8d"];
		cardsArray[18] = cardStringToInt["3d"];

		// player 8
		cardsArray[19] = cardStringToInt["9h"];
		cardsArray[20] = cardStringToInt["4h"];

		// player 9
		cardsArray[21] = cardStringToInt["2s"];
		cardsArray[22] = cardStringToInt["5s"];

		// player 10
		cardsArray[23] = cardStringToInt["3s"];
		cardsArray[24] = cardStringToInt["7c"];

	}

#endif

	int tempBoardArray[5];
	int tempPlayerArray[2];
	int tempPlayerAndBoardArray[7];
	int bestHandPos[5];
	int sBluff;
	for(i=0; i<5; i++) {
		tempBoardArray[i] = cardsArray[i];
		tempPlayerAndBoardArray[i+2] = cardsArray[i];
	}

	k = 0;

	myBoard->setCards(tempBoardArray);

	for(it=activePlayerList->begin(); it!=activePlayerList->end(); ++it, k++) {

		(*it)->getBestHandPosition(bestHandPos);

		for(j=0; j<2; j++) {
			tempPlayerArray[j] = cardsArray[2*k+j+5];
			tempPlayerAndBoardArray[j] = cardsArray[2*k+j+5];
		}

		(*it)->setCards(tempPlayerArray);
		(*it)->setCardsValueInt(CardsValue::cardsValue(tempPlayerAndBoardArray, bestHandPos));
		(*it)->setBestHandPosition(bestHandPos);
		(*it)->setRoundStartCash((*it)->getCash());
		(*it)->getCurrentHandActions().reset();
		(*it)->setPosition();
		(*it)->setEstimatedRange("");

		// error-check
		for(j=0; j<5; j++) {
			if (bestHandPos[j] == -1) {
				LOG_ERROR(__FILE__ << " (" << __LINE__ << "): ERROR getBestHandPosition");
			}
		}
	}

	preflopLastRaiserID = -1;

	// determine dealer, SB, BB

	assignButtons();

	setBlinds();

	myBettingRound = myFactory->createBettingRound(this, dealerPosition, smallBlind);


}



Hand::~Hand()
{
}

void Hand::start()
{

	//Log blinds sets for new Hand
	PlayerListConstIterator it_sB, it_bB;
	it_sB = getActivePlayerIt(getCurrentBettingRound()->getSmallBlindPositionId());
	it_bB = getActivePlayerIt(getCurrentBettingRound()->getBigBlindPositionId());
	if(it_sB != getActivePlayerList()->end() && it_bB != getActivePlayerList()->end()) {
		myGui->logNewBlindsSetsMsg((*it_sB)->getSet(), (*it_bB)->getSet(), (*it_sB)->getName().c_str(), (*it_bB)->getName().c_str());
	} else {
		LOG_ERROR(__FILE__ << " (" << __LINE__ << "): Log Error: cannot find sBID or bBID");
	}

	// deal cards
	myGui->dealHoleCards();

	getBoard()->collectSets();
	getGuiInterface()->refreshPot();

	// change rounds | first start preflop
	myGui->nextPlayerAnimation();
}

void Hand::assignButtons()
{

	size_t i;
	PlayerListIterator it;
	PlayerListConstIterator it_c;

	// delete all buttons
	for (it=seatsList->begin(); it!=seatsList->end(); ++it) {
		(*it)->setButton(BUTTON_NONE);
	}

	// assign dealer button
	it = getSeatIt(dealerPosition);
	if(it == seatsList->end()) {
		throw Exception(__FILE__, __LINE__, ERR_SEAT_NOT_FOUND);
	}
	(*it)->setButton(BUTTON_DEALER);


	// assign Small Blind next to dealer. ATTENTION: in heads up it is big blind
	// assign big blind next to small blind. ATTENTION: in heads up it is small blind
	bool nextActivePlayerFound = false;
	PlayerListIterator dealerPositionIt = getSeatIt(dealerPosition);
	if(dealerPositionIt == seatsList->end()) {
		throw Exception(__FILE__, __LINE__, ERR_SEAT_NOT_FOUND);
	}

	for(i=0; i<seatsList->size(); i++) {

		++dealerPositionIt;
		if(dealerPositionIt == seatsList->end()) dealerPositionIt = seatsList->begin();

		it = getActivePlayerIt( (*dealerPositionIt)->getID() );
		if(it != activePlayerList->end() ) {
			nextActivePlayerFound = true;
			if(activePlayerList->size() > 2) {
				//small blind normal
				(*it)->setButton(2);
				smallBlindPosition = (*it)->getID();
			} else {
				//big blind in heads up
				(*it)->setButton(3);
				bigBlindPosition = (*it)->getID();
				// lastPlayerAction for showing cards
			}

			// first player after dealer have to show his cards first (in showdown)
			lastActionPlayerID = (*it)->getID();
			myBoard->setLastActionPlayerID(lastActionPlayerID);

			++it;
			if(it == activePlayerList->end()) it = activePlayerList->begin();

			if(activePlayerList->size() > 2) {
				//big blind normal
				(*it)->setButton(3);
				bigBlindPosition = (*it)->getID();
			} else {
				//small blind in heads up
				(*it)->setButton(2);
				smallBlindPosition = (*it)->getID();
			}

			break;
		}

	}
	if(!nextActivePlayerFound) {
		throw Exception(__FILE__, __LINE__, ERR_NEXT_ACTIVE_PLAYER_NOT_FOUND);
	}

//        cout << "lAP-Button: " << lastActionPlayer << endl;
}

void Hand::setBlinds()
{

	PlayerListConstIterator it_c;

	//do sets --> TODO switch?
	for (it_c=runningPlayerList->begin(); it_c!=runningPlayerList->end(); ++it_c) {

		//small blind
		if((*it_c)->getButton() == BUTTON_SMALL_BLIND) {

			// All in ?
			if((*it_c)->getCash() <= smallBlind) {

				(*it_c)->setSet((*it_c)->getCash());
				// 1 to do not log this
				(*it_c)->setAction(PLAYER_ACTION_ALLIN,1);

			} else {
				(*it_c)->setSet(smallBlind);
			}
		}

	}

	//do sets --> TODO switch?
	for (it_c=runningPlayerList->begin(); it_c!=runningPlayerList->end(); ++it_c) {

		//big blind
		if((*it_c)->getButton() == BUTTON_BIG_BLIND) {

			// all in ?
			if((*it_c)->getCash() <= 2*smallBlind) {

				(*it_c)->setSet((*it_c)->getCash());
				// 1 to do not log this
				(*it_c)->setAction(PLAYER_ACTION_ALLIN,1);

			} else {
				(*it_c)->setSet(2*smallBlind);
			}
		}
	}
}


void Hand::switchRounds()
{

	PlayerListIterator it, it_1;
	PlayerListConstIterator it_c;

	// refresh runningPlayerList
	for(it=runningPlayerList->begin(); it!=runningPlayerList->end(); ) {
		if((*it)->getAction() == PLAYER_ACTION_FOLD || (*it)->getAction() == PLAYER_ACTION_ALLIN) {

			it = runningPlayerList->erase(it);
			if(!(runningPlayerList->empty())) {

				it_1 = it;
				if(it_1 == runningPlayerList->begin()) it_1 = runningPlayerList->end();
				--it_1;
				getCurrentBettingRound()->setCurrentPlayersTurnId((*it_1)->getID());

			}
		} else {
			++it;
		}
	}

	// determine number of all in players
	int allInPlayersCounter = 0;
	for (it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); ++it_c) {
		if ((*it_c)->getAction() == PLAYER_ACTION_ALLIN) allInPlayersCounter++;
	}

	// determine number of non-fold players
	int nonFoldPlayerCounter = 0;
	for (it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); ++it_c) {
		if ((*it_c)->getAction() != PLAYER_ACTION_FOLD) nonFoldPlayerCounter++;
	}

	// if only one player non-fold -> distribute pot
	if(nonFoldPlayerCounter==1) {
		myBoard->collectPot();
		myGui->refreshPot();
		myGui->refreshSet();
		currentRound = GAME_STATE_POST_RIVER;
		if(myLog) myLog->setCurrentRound(currentRound);
	}

	// check for all in condition
	// for all in condition at least two active players have to remain
	else {

		// 1) all players all in
		if(allInPlayersCounter == nonFoldPlayerCounter) {
			allInCondition = true;
			myBoard->setAllInCondition(true);
		}

		// 2) all players but one all in and he has highest set
		if(allInPlayersCounter+1 == nonFoldPlayerCounter) {

			for(it_c=runningPlayerList->begin(); it_c!=runningPlayerList->end(); ++it_c) {

				if((*it_c)->getSet() >= myBettingRound[currentRound]->getHighestSet()) {
					allInCondition = true;
					myBoard->setAllInCondition(true);
				}

			}

			// exception
			// no.1: if in first Preflop Round next player is small blind and only all-in-big-blind with less than smallblind amount and other all-in players with less than small blind are nonfold too -> preflop is over
			PlayerListConstIterator smallBlindIt_c = getRunningPlayerIt(myBettingRound[currentRound]->getSmallBlindPositionId());
			PlayerListConstIterator bigBlindIt_c = getActivePlayerIt(myBettingRound[currentRound]->getBigBlindPositionId());
			if(smallBlindIt_c!=runningPlayerList->end() &&  bigBlindIt_c!=activePlayerList->end() && currentRound == GAME_STATE_PREFLOP && myBettingRound[currentRound]->getFirstRound()) {
				// determine player who are all in with less than small blind amount
				int tempCounter = 0;
				for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); ++it_c) {
					if((*it_c)->getAction() == PLAYER_ACTION_ALLIN && (*it_c)->getSet() <= smallBlind) {
						tempCounter++;
					}
				}
				if( (*bigBlindIt_c)->getSet() <= smallBlind  && tempCounter == allInPlayersCounter) {
					allInCondition = true;
					myBoard->setAllInCondition(true);
				}
			}

			// no.2: heads up -> detect player who is all in and bb but could set less than sb
			for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); ++it_c) {

				if(activePlayerList->size()==2 && (*it_c)->getAction() == PLAYER_ACTION_ALLIN && (*it_c)->getButton()==BUTTON_BIG_BLIND && (*it_c)->getSet()<=smallBlind && currentRound == GAME_STATE_PREFLOP) {
					allInCondition = true;
					myBoard->setAllInCondition(true);
				}
			}
		}
	}

	// special routine
	if(allInCondition) {
		myBoard->collectPot();
		myGui->refreshPot();
		myGui->refreshSet();
		myGui->flipHolecardsAllIn();

		if (currentRound < GAME_STATE_POST_RIVER) { // do not increment past 4
			currentRound = GameState(currentRound + 1);
			if(myLog) myLog->setCurrentRound(currentRound);
		}

		//log board cards for allin
		if(currentRound >= GAME_STATE_FLOP) {
			int tempBoardCardsArray[5];

			myBoard->getCards(tempBoardCardsArray);
			myGui->logDealBoardCardsMsg(currentRound, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3], tempBoardCardsArray[4]);
		}

	}

	//unhighlight current players groupbox
	it_c = getActivePlayerIt(previousPlayerID);
	if( it_c != activePlayerList->end() ) {
		// lastPlayersTurn is active
		myGui->refreshGroupbox(previousPlayerID,1);
	}

	myGui->refreshGameLabels((GameState)getCurrentRound());

	if(currentRound < GAME_STATE_POST_RIVER) {
		roundBeforePostRiver = currentRound;
	}

	switch(currentRound) {
		case GAME_STATE_PREFLOP: {
			myGui->preflopAnimation1();
		}
		break;
		case GAME_STATE_FLOP: {
			myGui->flopAnimation1();
		}
		break;
		case GAME_STATE_TURN: {
			myGui->turnAnimation1();
		}
		break;
		case GAME_STATE_RIVER: {
			myGui->riverAnimation1();
		}
		break;
		case GAME_STATE_POST_RIVER: {
			myGui->postRiverAnimation1();
		}
		break;
		default:{}

	}

}

PlayerListIterator Hand::getSeatIt(unsigned uniqueId) const
{

	PlayerListIterator it;

	for(it=seatsList->begin(); it!=seatsList->end(); ++it) {
		if((*it)->getID() == uniqueId) {
			break;
		}
	}

	return it;

}

PlayerListIterator Hand::getActivePlayerIt(unsigned uniqueId) const
{

	PlayerListIterator it;

	for(it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {
		if((*it)->getID() == uniqueId) {
			break;
		}
	}

	return it;

}

PlayerListIterator Hand::getRunningPlayerIt(unsigned uniqueId) const
{

	PlayerListIterator it;

	for(it=runningPlayerList->begin(); it!=runningPlayerList->end(); ++it) {

		if((*it)->getID() == uniqueId) {
			break;
		}
	}

	return it;

}

void Hand::setLastActionPlayerID(unsigned theValue)
{
	lastActionPlayerID = theValue;
	myBoard->setLastActionPlayerID(theValue);
}

int Hand::getPreflopCallsNumber(){
	
	int calls = 0;

	for(PlayerListIterator it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {

		const std::vector<PlayerAction> & actions = (*it)->getCurrentHandActions().getPreflopActions();

		if (find(actions.begin(), actions.end(), PLAYER_ACTION_CALL) != actions.end()) 
			calls++;
	}
	return calls;
}
int Hand::getPreflopRaisesNumber(){
	
	int raises = 0;

	for(PlayerListIterator it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {

		const std::vector<PlayerAction> & actions = (*it)->getCurrentHandActions().getPreflopActions();

		for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
			if	((*itAction) == PLAYER_ACTION_RAISE || (*itAction) == PLAYER_ACTION_ALLIN) 
				raises++;
	}

	return raises;
}
int Hand::getFlopBetsOrRaisesNumber(){
	
	int bets = 0;

	for(PlayerListIterator it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {

		const std::vector<PlayerAction> & actions = (*it)->getCurrentHandActions().getFlopActions();

		for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
			if	((*itAction) == PLAYER_ACTION_RAISE || 
				 (*itAction) == PLAYER_ACTION_ALLIN ||
				 (*itAction) == PLAYER_ACTION_BET) 
				bets++;
	}

	return bets;
}
int Hand::getTurnBetsOrRaisesNumber(){
	
	int bets = 0;

	for(PlayerListIterator it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {

		const std::vector<PlayerAction> & actions = (*it)->getCurrentHandActions().getTurnActions();

		for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
			if	((*itAction) == PLAYER_ACTION_RAISE || 
				 (*itAction) == PLAYER_ACTION_ALLIN ||
				 (*itAction) == PLAYER_ACTION_BET) 
				bets++;
	}

	return bets;
}
int Hand::getRiverBetsOrRaisesNumber(){
	
	int bets = 0;

	for(PlayerListIterator it=activePlayerList->begin(); it!=activePlayerList->end(); ++it) {

		const std::vector<PlayerAction> & actions = (*it)->getCurrentHandActions().getRiverActions();

		for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end(); itAction++)
			if	((*itAction) == PLAYER_ACTION_RAISE || 
				 (*itAction) == PLAYER_ACTION_ALLIN ||
				 (*itAction) == PLAYER_ACTION_BET) 
				bets++;
	}

	return bets;
}
std::vector<PlayerPosition> Hand::getRaisersPositions(){

	std::vector<PlayerPosition> positions;

	PlayerListIterator it_c;

	for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); ++it_c) {// note that all in players are not "running" any more

		if((*it_c)->getAction() == PLAYER_ACTION_RAISE || (*it_c)->getAction() == PLAYER_ACTION_ALLIN) 
			positions.push_back((*it_c)->getPosition());
	}
	return positions;
}

std::vector<PlayerPosition> Hand::getCallersPositions(){

	std::vector<PlayerPosition> positions;

	PlayerListIterator it_c;

	for(it_c=runningPlayerList->begin(); it_c!=runningPlayerList->end(); ++it_c) {

		if((*it_c)->getAction() == PLAYER_ACTION_CALL) 
			positions.push_back((*it_c)->getPosition());
	}
	return positions;
}
int Hand::getLastRaiserID(){

	PlayerListIterator lastRaiser = activePlayerList->end();

	PlayerList players = activePlayerList;

	for(PlayerListIterator it=players->begin(); it!=players->end(); ++it) {

		if((*it)->getAction() == PLAYER_ACTION_RAISE || (*it)->getAction() == PLAYER_ACTION_ALLIN) {

			if (lastRaiser != activePlayerList->end()){
				if ((*lastRaiser)->getPosition() < (*it)->getPosition())
					lastRaiser = it;
			}else{
				lastRaiser = it;
			}
		}
	}
	if (lastRaiser != activePlayerList->end())
		return (*lastRaiser)->getID();

	// if no raiser was found, look for the one who have bet (if any)

	for(PlayerListIterator it=players->begin(); it!=players->end(); ++it) {

		if((*it)->getAction() == PLAYER_ACTION_BET) 
			lastRaiser = it;
	}
	if (lastRaiser != activePlayerList->end())
		return (*lastRaiser)->getID();
	else
		return -1;
}
int Hand::getPreflopLastRaiserID(){
	return preflopLastRaiserID;
}

void Hand::setPreflopLastRaiserID( int id){
	preflopLastRaiserID = id;
}
int Hand::getFlopLastRaiserID(){
	return flopLastRaiserID;
}

void Hand::setFlopLastRaiserID( int id){
	flopLastRaiserID = id;
}
int Hand::getTurnLastRaiserID(){
	return turnLastRaiserID;
}

void Hand::setTurnLastRaiserID( int id){
	turnLastRaiserID = id;
}

