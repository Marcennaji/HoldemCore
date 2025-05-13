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

#include "BettingRound.h"

#include "Exception.h"
#include "EngineError.h"
#include "ButtonState.h"

#include <core/engine/EngineServices.h>
#include <core/player/Player.h>

using namespace std;

BettingRound::BettingRound(EngineServices& services, HandInterface* hi, unsigned dP, int sB, GameState gS)
	: myEngineServices(services), BettingRoundInterface(), myHand(hi), myBettingRoundID(gS), dealerPosition(dP), smallBlindPosition(0), 
	dealerPositionId(dP), smallBlindPositionId(0), bigBlindPositionId(0), smallBlind(sB), 
	highestSet(0), minimumRaise(2*sB), fullBetRule(false), firstRun(true), firstRunGui(true), 
	firstRound(true), firstHeadsUpRound(true), currentPlayersTurnId(0), 
	firstRoundLastPlayersTurnId(0), logBoardCardsDone(false)
{
	currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();
	lastPlayersTurnIt = myHand->getRunningPlayerList()->begin();

	PlayerListConstIterator it_c;
	
	// determine bigBlindPosition
	for(it_c=myHand->getActivePlayerList()->begin(); it_c!=myHand->getActivePlayerList()->end(); ++it_c) {
		if((*it_c)->getButton() == BUTTON_BIG_BLIND) {
			bigBlindPositionId = (*it_c)->getID();
			break;
		}
	}
	if(it_c == myHand->getActivePlayerList()->end()) {
		myEngineServices.logger->error("ACTIVE_PLAYER_NOT_FOUND");
	}

	// determine smallBlindPosition
	for(it_c=myHand->getActivePlayerList()->begin(); it_c!=myHand->getActivePlayerList()->end(); ++it_c) {
		if((*it_c)->getButton() == BUTTON_SMALL_BLIND) {
			smallBlindPositionId = (*it_c)->getID();
			break;
		}
	}
	if(it_c == myHand->getActivePlayerList()->end()) {
		myEngineServices.logger->error("ACTIVE_PLAYER_NOT_FOUND");
	}

}


BettingRound::~BettingRound()
{
}

int BettingRound::getHighestCardsValue() const
{
	myEngineServices.logger->error("getHighestCardsValue() in wrong BettingRound");
	return 0;
}

void BettingRound::nextPlayer()
{

	PlayerListConstIterator currentPlayersTurnConstIt = myHand->getRunningPlayerIt(currentPlayersTurnId);
	if(currentPlayersTurnConstIt == myHand->getRunningPlayerList()->end()) {
		myEngineServices.logger->error("RUNNING_PLAYER_NOT_FOUND");
	}

	(*currentPlayersTurnConstIt)->action();

}

void BettingRound::run()
{

	if(firstRunGui) {
		firstRunGui = false;
		myHand->setPreviousPlayerID(-1);
		myHand->getGuiInterface()->dealBettingRoundCards(myBettingRoundID);
	} else {

		if(firstRun) {

			firstRun = false;

			if(!(myHand->getAllInCondition())) {

				PlayerListIterator it_1, it_2;

				// running player before smallBlind
				bool formerRunningPlayerFound = false;
				if(myHand->getActivePlayerList()->size() > 2) {

					it_1 = myHand->getActivePlayerIt(smallBlindPositionId);
					if(it_1 == myHand->getActivePlayerList()->end()) {
						myEngineServices.logger->error("ACTIVE_PLAYER_NOT_FOUND");
					}

					for(size_t i=0; i<myHand->getActivePlayerList()->size(); i++) {

						if(it_1 == myHand->getActivePlayerList()->begin()) it_1 = myHand->getActivePlayerList()->end();
						--it_1;

						it_2 = myHand->getRunningPlayerIt((*it_1)->getID());
						// running player found
						if(it_2 != myHand->getRunningPlayerList()->end()) {
							firstRoundLastPlayersTurnId = (*it_2)->getID();
							formerRunningPlayerFound = true;
							break;
						}
					}
					if(!formerRunningPlayerFound) {
						myEngineServices.logger->error("FORMER_RUNNING_PLAYER_NOT_FOUND");
					}
				}
				// heads up: bigBlind begins -> dealer/smallBlind is running player before bigBlind
				else {
					firstRoundLastPlayersTurnId = smallBlindPositionId;
				}
				currentPlayersTurnId = firstRoundLastPlayersTurnId;
			}
		}

		//log the turned cards
		if(!logBoardCardsDone) {

			int tempBoardCardsArray[5];

			myHand->getBoard()->getCards(tempBoardCardsArray);

			switch(myBettingRoundID) {
			case GAME_STATE_FLOP:
#ifdef LOG_POKER_EXEC
				cout << endl << endl << "************************* FLOP " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[0]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[1]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[2]]
					<< "  *************************" << endl << endl;
#endif
				myHand->getGuiInterface()->logDealBoardCardsMsg(myBettingRoundID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2]);
				break;
			case GAME_STATE_TURN:
#ifdef LOG_POKER_EXEC
				cout << endl << endl << "************************* TURN " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[0]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[1]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[2]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[3]]
					<< "  *************************" << endl << endl;
#endif
				myHand->getGuiInterface()->logDealBoardCardsMsg(myBettingRoundID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3]);
				break;
			case GAME_STATE_RIVER:
#ifdef LOG_POKER_EXEC
				cout << endl << endl << "************************* RIVER " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[0]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[1]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[2]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[3]] << " " 
					<< CardsValue::CardStringValue[tempBoardCardsArray[4]]
					<< "  *************************" << endl << endl;
#endif
				myHand->getGuiInterface()->logDealBoardCardsMsg(myBettingRoundID, tempBoardCardsArray[0], tempBoardCardsArray[1], tempBoardCardsArray[2], tempBoardCardsArray[3], tempBoardCardsArray[4]);

				break;
			default: 
				myEngineServices.logger->error("wrong myBettingRoundID");
			
			}
			logBoardCardsDone = true;
			
		}

		bool allHighestSet = true;

		PlayerListIterator it;
		PlayerListIterator it_c;

		for( it_c = myHand->getRunningPlayerList()->begin(); it_c != myHand->getRunningPlayerList()->end(); ++it_c) {			
			if(highestSet != (*it_c)->getSet())
				allHighestSet = false;
		}

		if(!firstRound && allHighestSet) {

			// go to next betting round
			myHand->setCurrentRound(GameState(myBettingRoundID+1));

			for(it_c=myHand->getRunningPlayerList()->begin(); it_c!=myHand->getRunningPlayerList()->end(); ++it_c) {
				(*it_c)->setAction(PLAYER_ACTION_NONE);
			}

			//Sets in den Pot verschieben und Sets = 0 und Pot-refresh
			myHand->getBoard()->collectSets();
			myHand->getBoard()->collectPot();
			myHand->getGuiInterface()->refreshPot();

			myHand->getGuiInterface()->refreshSet();
			myHand->getGuiInterface()->refreshCash();
			for(int i=0; i<MAX_NUMBER_OF_PLAYERS; i++) {
				myHand->getGuiInterface()->refreshAction(i,PLAYER_ACTION_NONE);
			}

			myHand->switchRounds();
		} else {
			// aktuelle BettingRound ist wirklich dran

			// determine next running player
			PlayerListConstIterator currentPlayersTurnIt = myHand->getRunningPlayerIt( currentPlayersTurnId );
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) {
				myEngineServices.logger->error("RUNNING_PLAYER_NOT_FOUND");
			}

			++currentPlayersTurnIt;
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) 
				currentPlayersTurnIt = myHand->getRunningPlayerList()->begin();

			currentPlayersTurnId = (*currentPlayersTurnIt)->getID();

			//highlight active players groupbox and clear action
			myHand->getGuiInterface()->refreshGroupbox(currentPlayersTurnId,2);
			myHand->getGuiInterface()->refreshAction(currentPlayersTurnId,0);

			currentPlayersTurnIt = myHand->getRunningPlayerIt( currentPlayersTurnId );
			if(currentPlayersTurnIt == myHand->getRunningPlayerList()->end()) {
				myEngineServices.logger->error("RUNNING_PLAYER_NOT_FOUND");
			}

			(*currentPlayersTurnIt)->setTurn(true);


			if( currentPlayersTurnId == firstRoundLastPlayersTurnId ) {
				firstRound = false;
			}

			if( currentPlayersTurnId == 0) {
				// Wir sind dran
				myHand->getGuiInterface()->meInAction();
			} else {

				//Gegner sind dran
				myHand->getGuiInterface()->beRoAnimation2(myBettingRoundID);
			}

		}
	}
}
GameState BettingRound::getBettingRoundID() const {
	return myBettingRoundID;
}

void BettingRound::setHighestCardsValue(int /*theValue*/) { }

void BettingRound::setMinimumRaise ( int theValue ) {
	minimumRaise = theValue;
}
int BettingRound::getMinimumRaise() const {
	return minimumRaise;
}

void BettingRound::setFullBetRule ( bool theValue ) {
	fullBetRule = theValue;
}
bool BettingRound::getFullBetRule() const {
	return fullBetRule;
}

void BettingRound::skipFirstRunGui() {
	firstRunGui = false;
}

void BettingRound::postRiverRun() {};

HandInterface* BettingRound::getHand() const {
	return myHand;
}

int BettingRound::getDealerPosition() const {
	return dealerPosition;
}
void BettingRound::setDealerPosition(int theValue) {
	dealerPosition = theValue;
}

void BettingRound::setCurrentPlayersTurnId(unsigned theValue) {
	currentPlayersTurnId = theValue;
}
unsigned BettingRound::getCurrentPlayersTurnId() const {
	return currentPlayersTurnId;
}

void BettingRound::setFirstRoundLastPlayersTurnId(unsigned theValue) {
	firstRoundLastPlayersTurnId = theValue;
}
unsigned BettingRound::getFirstRoundLastPlayersTurnId() const {
	return firstRoundLastPlayersTurnId;
}

void BettingRound::setCurrentPlayersTurnIt(PlayerListIterator theValue) {
	currentPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getCurrentPlayersTurnIt() const {
	return currentPlayersTurnIt;
}

void BettingRound::setLastPlayersTurnIt(PlayerListIterator theValue) {
	lastPlayersTurnIt = theValue;
}
PlayerListIterator BettingRound::getLastPlayersTurnIt() const {
	return lastPlayersTurnIt;
}

void BettingRound::setHighestSet(int theValue) {
	highestSet = theValue;
}
int BettingRound::getHighestSet() const {
	return highestSet;
}

void BettingRound::setFirstRun(bool theValue) {
	firstRun = theValue;
}
bool BettingRound::getFirstRun() const {
	return firstRun;
}

void BettingRound::setFirstRound(bool theValue) {
	firstRound = theValue;
}
bool BettingRound::getFirstRound() const {
	return firstRound;
}


void BettingRound::setDealerPositionId(unsigned theValue) {
	dealerPositionId = theValue;
}
unsigned BettingRound::getDealerPositionId() const {
	return dealerPositionId;
}

void BettingRound::setSmallBlindPositionId(unsigned theValue) {
	smallBlindPositionId = theValue;
}
unsigned BettingRound::getSmallBlindPositionId() const {
	return smallBlindPositionId;
}

void BettingRound::setBigBlindPositionId(unsigned theValue) {
	bigBlindPositionId = theValue;
}
unsigned BettingRound::getBigBlindPositionId() const {
	return bigBlindPositionId;
}


void BettingRound::setSmallBlindPosition(int theValue) {
	smallBlindPosition = theValue;
}
int BettingRound::getSmallBlindPosition() const {
	return smallBlindPosition;
}

void BettingRound::setSmallBlind(int theValue) {
	smallBlind = theValue;
}
int BettingRound::getSmallBlind() const {
	return smallBlind;
}