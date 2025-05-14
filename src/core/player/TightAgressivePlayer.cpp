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
#include "TightAgressivePlayer.h"

#include <core/engine/HandInterface.h>
#include <core/engine/Tools.h>
#include <core/engine/CardsValue.h>
#include <core/engine/EngineError.h>
#include <core/engine/Ranges.h>
#include <core/engine/Exception.h>
#include <core/interfaces/ILogger.h>
#include <ui/qtwidgets/guiwrapper.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

using namespace std;

TightAgressivePlayer::TightAgressivePlayer(SqliteLogStore * l, int id, PlayerType type, std::string name, int sC, bool aS, bool sotS, int mB):
	Player(l, id, type, name, sC, aS, sotS, mB){
	
	int utgFullTableRange = 0;
	Tools::GetRand(2, 3, 1, &utgFullTableRange);
	initializeRanges(45, utgFullTableRange);
}

	
TightAgressivePlayer::~TightAgressivePlayer(){}

bool TightAgressivePlayer::preflopShouldCall(){

	const int nbRaises = currentHand->getPreflopRaisesNumber();
	const int nbCalls = currentHand->getPreflopCallsNumber();
	const int nbPlayers = currentHand->getActivePlayerList()->size(); 

	float callingRange = getPreflopCallingRange();

	if (callingRange == -1)
		return false;// never call : raise or fold

	string stringCallingRange;

	const char * * RANGES_STRING;

	if (nbPlayers == 2)
		RANGES_STRING = TOP_RANGE_2_PLAYERS;
	else if (nbPlayers == 3)
		RANGES_STRING = TOP_RANGE_3_PLAYERS;
	else if (nbPlayers == 4)
		RANGES_STRING = TOP_RANGE_4_PLAYERS;
	else RANGES_STRING = TOP_RANGE_MORE_4_PLAYERS;

	stringCallingRange = RANGES_STRING[(int)callingRange];

	const int lastRaiserID = currentHand->getPreflopLastRaiserID();
	std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
	
	if (currentHand->getRunningPlayerList()->size() > 2 && 
		nbRaises + nbCalls > 1 && 
		nbRaises == 1 &&
		myPosition >= LATE &&
		getCash() >= currentHand->getBoard()->getPot() * 10 &&
		lastRaiser != NULL && 
		lastRaiser->getCash() >= currentHand->getBoard()->getPot() * 20 &&
		! isPreflopBigBet()){

		stringCallingRange += HIGH_SUITED_CONNECTORS;

#ifdef LOG_POKER_EXEC
	cout << "\t\tTAG adding high suited connectors to the initial calling range." << endl;
#endif
	}

	// defend against 3bet bluffs :
	if (nbRaises == 2 &&
		myCurrentHandActions.getPreflopActions().size() > 0 &&
		myCurrentHandActions.getPreflopActions().back() == PLAYER_ACTION_RAISE &&
		getCash() >= currentHand->getBoard()->getPot() * 10 &&
		lastRaiser != NULL && 
		lastRaiser->getCash() >= currentHand->getBoard()->getPot() * 10 &&
		! isPreflopBigBet()){

		int rand = 0;
		Tools::GetRand(1, 4, 1, &rand);
		if (rand == 1){

			stringCallingRange += HIGH_SUITED_CONNECTORS;
			stringCallingRange += HIGH_SUITED_ACES;
			stringCallingRange += PAIRS;

	#ifdef LOG_POKER_EXEC
			cout << "\t\TAG defending against 3-bet : adding high suited connectors, high suited aces and pairs to the initial calling range." << endl;
	#endif
		}
	}
#ifdef LOG_POKER_EXEC
	cout << "\t\tTAG final calling range is " << stringCallingRange << endl;
#endif

	return isCardsInRange(myCard1, myCard2, stringCallingRange);

}

bool TightAgressivePlayer::preflopShouldRaise(){

	const int nbRaises = currentHand->getPreflopRaisesNumber();
	const int nbCalls = currentHand->getPreflopCallsNumber();
	const int nbPlayers = currentHand->getActivePlayerList()->size(); 

	float raisingRange = getPreflopRaisingRange();

	if (raisingRange == -1)
		return false;// never raise : call or fold
	
	if (nbRaises > 1)
		return false;// never 4-bet : call or fold

	string stringRaisingRange;

	const char * * RANGES_STRING;

	if (nbPlayers == 2)
		RANGES_STRING = TOP_RANGE_2_PLAYERS;
	else if (nbPlayers == 3)
		RANGES_STRING = TOP_RANGE_3_PLAYERS;
	else if (nbPlayers == 4)
		RANGES_STRING = TOP_RANGE_4_PLAYERS;
	else RANGES_STRING = TOP_RANGE_MORE_4_PLAYERS;

	stringRaisingRange = RANGES_STRING[(int)raisingRange];

#ifdef LOG_POKER_EXEC
	cout << stringRaisingRange << endl;
#endif

	// determine when to 3-bet without a real hand
	bool speculativeHandedAdded = false;

	if (nbRaises == 1){

		const int lastRaiserID = currentHand->getPreflopLastRaiserID();
		std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
		PreflopStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getPreflopStatistics();

		if (! isCardsInRange(myCard1, myCard2, stringRaisingRange) &&
			getM() > 20 &&
			myCash > currentHand->getCurrentBettingRound()->getHighestSet() * 20 &&
			myPosition > MIDDLE_PLUS_ONE &&
			raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE &&
			myPosition > lastRaiser->getPosition() &&
			lastRaiser->getCash() > currentHand->getCurrentBettingRound()->getHighestSet() * 10 &&
			! isPreflopBigBet() && 
			nbCalls < 2){

			if (canBluff(GAME_STATE_PREFLOP) &&
				myPosition > LATE && 
				! isCardsInRange(myCard1, myCard2, ACES + BROADWAYS) &&
				raiserStats.getPreflopCall3BetsFrequency() < 30){

				int rand = 0;
				Tools::GetRand(1, 2, 1, &rand);
				if (rand == 2){
					speculativeHandedAdded = true;
#ifdef LOG_POKER_EXEC
					cout << "\t\tTAG trying to steal this bet";
#endif
				}
			}else{
				if (isCardsInRange(myCard1, myCard2, LOW_PAIRS + CONNECTORS + SUITED_ONE_GAPED + SUITED_TWO_GAPED) &&
					raiserStats.getPreflopCall3BetsFrequency() < 30){ 

					speculativeHandedAdded = true;
#ifdef LOG_POKER_EXEC
					cout << "\t\tTAG adding this speculative hand to our initial raising range";
#endif
				}
			}
		}
	}

	if (! speculativeHandedAdded && ! isCardsInRange(myCard1, myCard2, stringRaisingRange))
		return false;

	// sometimes, just call a single raise instead of raising, even with a strong hand
	if (! speculativeHandedAdded && 
		currentHand->getPreflopCallsNumber() == 0 &&
		currentHand->getPreflopRaisesNumber() == 1 &&
		raisingRange < 100 && 
		! (isCardsInRange(myCard1, myCard2, LOW_PAIRS + MEDIUM_PAIRS) && nbPlayers < 4) &&
		! (isCardsInRange(myCard1, myCard2, HIGH_PAIRS) && nbCalls > 0) &&
		isCardsInRange(myCard1, myCard2, getStringRange(nbPlayers, 4))){

		int rand = 0;
		Tools::GetRand(1, 8, 1, &rand);
		if (rand == 1){
#ifdef LOG_POKER_EXEC
			cout << "\t\twon't raise, to hide the hand strength";
#endif
			myShouldCall = true;
			return false;
		}
	}

	computePreflopRaiseAmount();

	return true;
}



bool TightAgressivePlayer::flopShouldBet(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	const int nbPlayers = currentHand->getActivePlayerList()->size(); 
	const int lastRaiserID = currentHand->getPreflopLastRaiserID();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

	assert(nbPlayers >= 2);

	const int nbPreflopRaises = currentHand->getPreflopRaisesNumber();
	const int nbFlopRaises = currentHand->getFlopBetsOrRaisesNumber();

	if (nbFlopRaises > 0)
		return false;

	if (shouldPotControl(myFlopState, myFlopHandSimulation, GAME_STATE_FLOP))
		return false;

	// donk bets :
	if (nbPreflopRaises > 0 && lastRaiserID != myID){

		std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);

		if (lastRaiser->getPosition() > myPosition){

			if (getDrawingProbability() > 25){
				int rand = 0;
				Tools::GetRand(1, 2, 1, &rand);
				if (rand == 1){
					myBetAmount = pot * 0.6;
					return true;
				}
			}

			if ((myFlopState.IsTwoPair || myFlopState.IsTrips || myFlopState.IsStraight) && myFlopState.IsFlushDrawPossible){
				myBetAmount = pot * 0.6;
				return true;
			}

			// if the flop is dry, try to get the pot
			if (nbPlayers < 3 && canBluff(GAME_STATE_FLOP) && 
				getBoardCardsHigherThan("Jh") < 2 && getBoardCardsHigherThan("Kh") == 0 &&
				! myFlopState.IsFlushDrawPossible){

				int rand = 0;
				Tools::GetRand(1, 3, 1, &rand);
				if (rand == 1){
					myBetAmount = pot * 0.6;
					return true;
				}
			}
		}
	}

	// don't bet if in position, and pretty good drawing probs
	if (getDrawingProbability() > 20 && bHavePosition)
		return false;

	// if pretty good hand
	if (myFlopHandSimulation.winRanged > 0.6 || myFlopHandSimulation.win > 0.94){

		// always bet if my hand will lose a lot of its value on next betting rounds
		if (myFlopHandSimulation.winRanged - myFlopHandSimulation.winSd > 0.1 && bHavePosition){
			myBetAmount = pot;
			return true;
		}
		
		int rand = 0;
		Tools::GetRand(1, 7, 1, &rand);
		if (rand == 3 && ! bHavePosition && lastRaiserID != myID)
			return false;// may check-raise or check-call

		// if no raise preflop, or if more than 1 opponent
		if (currentHand->getPreflopRaisesNumber() == 0 || runningPlayers->size() > 2){

			if (runningPlayers->size() < 4)
				myBetAmount = pot * 0.6;
			else
				myBetAmount = pot;
			return true;
		}

		// if i have raised preflop, bet
		if (lastRaiserID == myID){

			if (runningPlayers->size() < 4)
				myBetAmount = pot * 0.6;
			else
				myBetAmount = pot;

			return true;
		}

	}else{

		///////////  if bad flop for me

		// if there was a lot of action preflop, and i was not the last raiser : don't bet
		if (nbPreflopRaises > 1 && lastRaiserID != myID)
			return false;

		// if I was the last raiser preflop, bet if i have a big enough stack
		if (lastRaiserID == myID && runningPlayers->size() < 4 && myCash > pot * 5 && canBluff(GAME_STATE_FLOP)){
			myBetAmount = pot * 0.6;		
			return true;
		}
	}

	return false;

}
bool TightAgressivePlayer::flopShouldCall(){

	const int nbRaises = currentHand->getFlopBetsOrRaisesNumber();

	if (nbRaises == 0)
		return false;

	if (isDrawingProbOk())
		return true;

	if (myFlopHandSimulation.winRanged == 1 && myFlopHandSimulation.win > 0.5)
		return true;

	if (myFlopHandSimulation.winRanged * 100 < getPotOdd() && myFlopHandSimulation.win < 0.94)
		return false;

	if (myFlopHandSimulation.winRanged < 0.25 && myFlopHandSimulation.win < 0.3)
		return false;

	return true; 
}

bool TightAgressivePlayer::flopShouldRaise(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
	const int potOdd = getPotOdd();
	const int lastRaiserID = currentHand->getPreflopLastRaiserID();
	std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
	const int nbPlayers = currentHand->getActivePlayerList()->size(); 
	assert(nbPlayers >= 2);

	const int nbRaises = currentHand->getFlopBetsOrRaisesNumber();

	if (nbRaises == 0)
		return false;

	if (shouldPotControl(myFlopState, myFlopHandSimulation, GAME_STATE_FLOP))
		return false;

	if (nbRaises == 1 && myFlopHandSimulation.win < 0.90)
		return false;

	if (nbRaises == 2 && myFlopHandSimulation.win < 0.95)
		return false;

	if (nbRaises == 3 && myFlopHandSimulation.win < 0.98)
		return false;

	if (nbRaises > 3 && myFlopHandSimulation.win != 1)
		return false;

	if ((isDrawingProbOk() || bHavePosition) && runningPlayers->size() == 2 &&
		! (myFlopHandSimulation.winRanged * 100 < getPotOdd()) &&
		canBluff(GAME_STATE_FLOP) &&
		nbRaises < 2){

		int rand = 0;
		Tools::GetRand(1, 4, 1, &rand);
		if (rand == 2){
			myRaiseAmount = pot;
			return true;
		}
	}

	if (myFlopHandSimulation.winRanged * 100 < getPotOdd()){

		if (getPotOdd() < 30 && runningPlayers->size() < 4){

			int rand = 0;
			Tools::GetRand(1, 6, 1, &rand);
			if (rand == 2 && myFlopHandSimulation.winRanged > 0.3 && myFlopHandSimulation.win > 0.3){
				myRaiseAmount = pot;
				return true;
			}
		}
		return false;
	}

	if (myFlopHandSimulation.winRanged > 0.9 && nbRaises < 3){
		myRaiseAmount = pot;
		return true;
	}
	if (myFlopHandSimulation.winRanged > 0.75 && nbRaises < 2){
		myRaiseAmount = pot;
		return true;
	}

	return false;
}

bool TightAgressivePlayer::turnShouldBet(){

	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	const int nbRaises = currentHand->getTurnBetsOrRaisesNumber();

	if (nbRaises > 0)
		return false;

	if (shouldPotControl(myTurnState, myTurnHandSimulation, GAME_STATE_TURN))
		return false;

	if (currentHand->getFlopBetsOrRaisesNumber() > 1 && ! isAgressor(GAME_STATE_FLOP) && myTurnHandSimulation.winRanged < 0.75 && myTurnHandSimulation.win < 0.9)
		return false;

	if (currentHand->getFlopBetsOrRaisesNumber() == 0 && 
		bHavePosition && 
		runningPlayers->size() < 4 && 
		getDrawingProbability() < 9 &&
		myCash > pot * 4){

		int rand = 0;
		Tools::GetRand(1, 2, 1, &rand);
		if (rand == 1){
			myBetAmount = pot * 0.6;
			return true;
		}
	}

	if (myCash < pot * 4 && myTurnHandSimulation.winRanged < 0.6 && myTurnHandSimulation.win < 0.9)
		return false;

	if (myTurnHandSimulation.winRanged < 0.6 && myTurnHandSimulation.win < 0.9 && ! bHavePosition)
		return false;

	if (myTurnHandSimulation.winRanged > 0.5 && myTurnHandSimulation.win > 0.7 && bHavePosition){
		myBetAmount = pot * 0.6;
		return true;
	}

	if (getDrawingProbability() > 15 && ! bHavePosition){
		int rand = 0;
		Tools::GetRand(1, 3, 1, &rand);
		if (rand == 1){
			myBetAmount = pot * 0.6;
			return true;
		}
	}else{
		// no draw, not a good hand, but last to speak and nobody has bet
		if (bHavePosition && canBluff(GAME_STATE_TURN)){
			int rand = 0;
			Tools::GetRand(1, 3, 1, &rand);
			if (rand == 2){
				myBetAmount = pot * 0.6;
				return true;
			}
		}
	}

	return false;
}

bool TightAgressivePlayer::turnShouldCall(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	std::vector<PlayerPosition> raisersPositions = currentHand->getRaisersPositions();
	std::vector<PlayerPosition> callersPositions = currentHand->getCallersPositions();
	const int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
	const int nbRaises = currentHand->getTurnBetsOrRaisesNumber();
	const int potOdd = getPotOdd();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
	const int lastRaiserID = currentHand->getLastRaiserID();
	std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
	const int nbPlayers = currentHand->getActivePlayerList()->size();
	
	if (nbRaises == 0)
		return false;

	if (isDrawingProbOk())
		return true;
	
	assert(lastRaiser != NULL);

	TurnStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getTurnStatistics();

	// if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
	if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 && 
		lastRaiser->getStatistics(nbPlayers + 1).getTurnStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)

		raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getTurnStatistics();

	if (myTurnHandSimulation.winRanged * 100 < getPotOdd() && myTurnHandSimulation.winRanged < 0.94){
		return false;
	}

	if (nbRaises == 2 && myTurnHandSimulation.winRanged < 0.8){
		if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
			return false;
		if (raiserStats.getAgressionFrequency() < 20)
			return false;
	}
	if (nbRaises > 2 && myTurnHandSimulation.winRanged < 0.9){
		if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE)
			return false;
		if (raiserStats.getAgressionFrequency() < 20)
			return false;
	}

	if (myTurnHandSimulation.winRanged < 0.6 && (currentHand->getFlopBetsOrRaisesNumber() > 0 || raiserStats.getAgressionFrequency() < 30))
		return false;

	if (! isAgressor(GAME_STATE_PREFLOP) && ! isAgressor(GAME_STATE_FLOP) 
		&& myTurnHandSimulation.winRanged < 0.8 && raiserStats.getAgressionFrequency() < 30 && ! bHavePosition)
		return false;

	if (myTurnHandSimulation.winRanged < 0.25)
		return false;

	return true;

}

bool TightAgressivePlayer::turnShouldRaise(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	std::vector<PlayerPosition> raisersPositions = currentHand->getRaisersPositions();
	std::vector<PlayerPosition> callersPositions = currentHand->getCallersPositions();
	const int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
	const int nbRaises = currentHand->getTurnBetsOrRaisesNumber();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

	if (nbRaises == 0)
		return false;

	if (shouldPotControl(myTurnState, myTurnHandSimulation, GAME_STATE_TURN))
		return false;

	if (nbRaises == 2 && myTurnHandSimulation.win < 0.98)
		return false;

	if (nbRaises > 2 && myTurnHandSimulation.win != 1)
		return false;

	if (myTurnHandSimulation.winRanged > 0.98 && myTurnHandSimulation.win > 0.98 && myTurnHandSimulation.winSd > 0.9){
		int rand = 0;
		Tools::GetRand(1, 3, 1, &rand);
		if (rand == 1)
			return false; // very strong hand, slow play, just call
	}

	if (myTurnHandSimulation.win == 1 || (myTurnHandSimulation.winRanged == 1 && nbRaises < 3)){
		myRaiseAmount = pot * 0.6;
		return true;
	}

	if (myTurnHandSimulation.winRanged * 100 < getPotOdd() && myTurnHandSimulation.winRanged < 0.94)
		return false;

	if (myTurnHandSimulation.winRanged > 0.9 && myTurnHandSimulation.win > 0.9 &&
		nbRaises == 1 && currentHand->getFlopBetsOrRaisesNumber() < 2){

		myRaiseAmount = pot* 0.6;
		return true;
	}
	if (myTurnHandSimulation.winRanged > 0.94 && myTurnHandSimulation.win > 0.94 && nbRaises < 4){
		myRaiseAmount = pot * 0.6;
		return true;
	}

	return false;
}

bool TightAgressivePlayer::riverShouldBet(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	std::vector<PlayerPosition> raisersPositions = currentHand->getRaisersPositions();
	std::vector<PlayerPosition> callersPositions = currentHand->getCallersPositions();
	const int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
	const int nbRaises = currentHand->getRiverBetsOrRaisesNumber();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());
	
	if (nbRaises > 0)
		return false;

	// blocking bet if my chances to win are weak, but not ridiculous
	if (! bHavePosition && myRiverHandSimulation.winRanged < .7 && myRiverHandSimulation.winRanged > .4 && myRiverHandSimulation.winSd > 0.4){
		int rand = 0;
		Tools::GetRand(1, 2, 1, &rand);
		if (rand == 1){
			myBetAmount = pot * 0.33;
			return true;
		}
	}

	// bluff if very few chances to win, and if I was the agressor on turn
	if (isAgressor(GAME_STATE_TURN)){

		if (myRiverHandSimulation.winRanged < .15  && 
			myRiverHandSimulation.winSd > 0.3 && 
			runningPlayers->size() < 4 && 
			getCash() >= currentHand->getBoard()->getPot() &&
			canBluff(GAME_STATE_RIVER)){ 
	
				int rand = 0;
				Tools::GetRand(1, 4, 1, &rand);
				if (rand == 1){
					myBetAmount = pot * 0.8;
					return true;
				}
		}
	}

	if (myRiverHandSimulation.winSd < .94 && currentHand->getTurnBetsOrRaisesNumber() > 0 && ! isAgressor(GAME_STATE_TURN))
		return false;

	int rand = 0;
	Tools::GetRand(40, 80, 1, &rand);
	float coeff = (float)rand / (float)100;

	if ( myRiverHandSimulation.winSd > .94 || (bHavePosition && myRiverHandSimulation.winSd > .9)){ 

		int rand = 0;
		Tools::GetRand(1, 6, 1, &rand);
		if (rand != 1  || bHavePosition){
			myBetAmount = pot * coeff;
			return true;
		}
	}
	if (myRiverHandSimulation.winSd > 0.5 && ( myRiverHandSimulation.winRanged > .9 || (bHavePosition && myRiverHandSimulation.winRanged > .8))){ 
		int rand = 0;
		Tools::GetRand(1, 3, 1, &rand);
		if (rand == 1 || bHavePosition){
			myBetAmount = pot * coeff;
			return true;
		}
	}

	if (bHavePosition && nbRaises == 0 && myRiverHandSimulation.winRanged < 0.3 && currentHand->getTurnBetsOrRaisesNumber() == 0){
		int rand = 0;
		Tools::GetRand(1, 6, 1, &rand);
		if (rand == 1){
			myBetAmount = pot * coeff;
			return true;
		}
	}
	if (bHavePosition && nbRaises == 0 && myRiverHandSimulation.winRanged > 0.7 && currentHand->getTurnBetsOrRaisesNumber() == 0){
		int rand = 0;
		Tools::GetRand(1, 3, 1, &rand);
		if (rand != 1){
			myBetAmount = pot * coeff;
			return true;
		}
	}
	return false;
}

bool TightAgressivePlayer::riverShouldCall(){

	const int lastRaiserID = currentHand->getLastRaiserID();
	std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
	const int nbPlayers = currentHand->getActivePlayerList()->size();
	const int nbRaises = currentHand->getRiverBetsOrRaisesNumber();

	if (nbRaises == 0)
		return false;
	
	assert(lastRaiser != NULL);

	RiverStatistics raiserStats = lastRaiser->getStatistics(nbPlayers).getRiverStatistics();

	// if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more accurate
	if (raiserStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 && lastRaiser->getStatistics(nbPlayers + 1).getTurnStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
		raiserStats = lastRaiser->getStatistics(nbPlayers + 1).getRiverStatistics();

	if (myRiverHandSimulation.winRanged * 100 < getPotOdd() && myRiverHandSimulation.winRanged < 0.94){
		if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getAgressionFrequency() < 40)
			return false;
	}

	if (myRiverHandSimulation.winRanged < .7 && nbRaises == 1){ 
		if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && lastRaiser->getStatistics(nbPlayers).getWentToShowDown() < 40)
			return false;
	}

	if (myRiverHandSimulation.winRanged < .9 && nbRaises > 1){ 
		if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && lastRaiser->getStatistics(nbPlayers).getWentToShowDown() < 40)
			return false;
	}

	// if hazardous call may cost me my stack, don't call even with good odds

	if (getPotOdd() >  10 && 
		myRiverHandSimulation.winRanged < .5 &&
		currentHand->getCurrentBettingRound()->getHighestSet() >= myCash + mySet &&
		getM() > 8){ 
		if (raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && lastRaiser->getStatistics(nbPlayers).getWentToShowDown() < 50)
			return false;
	}

	// assume that if there was more than 1 player to play after the raiser and he is not a maniac, he shouldn't bluff
	if (currentHand->getRunningPlayerList()->size() > 2 && 
		myRiverHandSimulation.winRanged < .6 &&
		(raiserStats.m_hands > MIN_HANDS_STATISTICS_ACCURATE && raiserStats.getAgressionFactor() < 4 && raiserStats.getAgressionFrequency() < 50)){

		PlayerListConstIterator it_c;
		int playersAfterRaiser = 0;
	
		for(it_c=currentHand->getRunningPlayerList()->begin(); it_c!=currentHand->getRunningPlayerList()->end(); ++it_c) {
			if((*it_c)->getPosition() > lastRaiser->getPosition()) {
				playersAfterRaiser++;
			}
		}
		if (playersAfterRaiser > 1)
			return false;
	}

	if (raiserStats.m_hands <= MIN_HANDS_STATISTICS_ACCURATE && getPotOdd() * 1.5 > myRiverHandSimulation.winRanged * 100)
		return false;

    return true;
}

bool TightAgressivePlayer::riverShouldRaise(){

	const int pot = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets();
	PlayerList runningPlayers = currentHand->getRunningPlayerList();
	std::vector<PlayerPosition> raisersPositions = currentHand->getRaisersPositions();
	std::vector<PlayerPosition> callersPositions = currentHand->getCallersPositions();
	const int highestSet = currentHand->getCurrentBettingRound()->getHighestSet();
	const int nbRaises = currentHand->getRiverBetsOrRaisesNumber();
	const bool bHavePosition = getHavePosition(myPosition, currentHand->getRunningPlayerList());

	if (nbRaises == 0)
		return false;

	//  raise if i have the nuts. TODO : analyze previous actions, and determine if we must bet for value, without the nuts
	if (nbRaises < 3 && myRiverHandSimulation.winRanged > .98 && myRiverHandSimulation.winSd > 0.5){

		myRaiseAmount = pot * 0.6;
		return true;
	}

	if (nbRaises < 2 && myRiverHandSimulation.winRanged * 100 > getPotOdd() && myRiverHandSimulation.winRanged > 0.9 && myRiverHandSimulation.winSd > 0.5){

		myRaiseAmount = pot * 0.6;
		return true;
	}

	return false;
}





