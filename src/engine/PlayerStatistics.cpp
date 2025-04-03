/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
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

#include "PlayerStatistics.h"

const int PreflopStatistics::LAST_ACTIONS_STACK_SIZE = 10;

const PreflopStatistics & PlayerStatistics::getPreflopStatistics() const{

	return m_preflopStatistics;

}
const FlopStatistics & PlayerStatistics::getFlopStatistics() const{
	return m_flopStatistics;
}

const TurnStatistics & PlayerStatistics::getTurnStatistics() const{
	return m_turnStatistics;
}
const RiverStatistics & PlayerStatistics::getRiverStatistics() const{
	return m_riverStatistics;
}

void PlayerStatistics::reset(){

	m_preflopStatistics.reset();
	m_flopStatistics.reset();
	m_turnStatistics.reset();
	m_riverStatistics.reset();
}

float PlayerStatistics::getWentToShowDown() const{

	if ((m_riverStatistics.m_hands - m_riverStatistics.m_folds) == 0)
		return 0;
		
	return ((m_riverStatistics.m_hands - m_riverStatistics.m_folds)  * 100) / (m_flopStatistics.m_hands);

}

void PreflopStatistics::AddLastAction(PlayerAction p){
	m_lastActions.push_back(p);
	if (m_lastActions.size() > LAST_ACTIONS_STACK_SIZE)
		m_lastActions.pop_front();
}
int PreflopStatistics::GetLastActionsNumber(PlayerAction p) const{

    int result = 0;
	for(int i = 0; i < m_lastActions.size(); i++)
		if (m_lastActions.at(i) == p)
			result++;

	return result;
}
void PreflopStatistics::reset(){
	
	m_hands = 0;
	m_folds = 0;
	m_checks = 0;
	m_calls = 0;
	m_raises = 0;
	m_limps = 0;
	m_3Bets = 0;
	m_call3Bets = 0;
	m_call3BetsOpportunities = 0;
	m_4Bets = 0;
}
float PreflopStatistics::getVoluntaryPutMoneyInPot() const{
	
	if (m_hands == 0 || (m_raises + m_calls == 0))
		return 0;

	float f = (float)((m_raises + m_calls) * 100) / (float)m_hands;

	return (f > 100 ? 100 : f);

}

float PreflopStatistics::getPreflopRaise() const{

	if (m_hands == 0 || m_raises == 0)
		return 0;

	float f = (float)((m_raises * 100) / (float)m_hands);

	return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflop3Bet() const{

	if (m_hands == 0 || m_3Bets == 0)
		return 0;

	float f = (float)((m_3Bets * 100) / (float)m_hands);

	return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflop4Bet() const{

	if (m_hands == 0 || m_4Bets == 0)
		return 0;

	float f = (float)((m_4Bets * 100) / (float)m_hands);

	return (f > 100 ? 100 : f);
}
float PreflopStatistics::getPreflopCall3BetsFrequency() const{

	if (m_call3BetsOpportunities == 0)
		return 0;

	float f = (float)(m_call3Bets) / (float)(m_call3BetsOpportunities) * 100;

	return (f > 100 ? 100 : f);
}

void FlopStatistics::reset(){
	
	m_hands = 0;
	m_folds = 0;
	m_checks = 0;
	m_bets = 0;
	m_calls = 0;
	m_raises = 0;
	m_3Bets = 0;
	m_4Bets = 0;
	m_continuationBetsOpportunities = 0;
	m_continuationBets = 0;
}
float FlopStatistics::getAgressionFactor() const{

	if (m_raises + m_bets == 0 || m_calls == 0)
		return 0;

	return (float)(m_raises + m_bets) / (float)m_calls;
}

float FlopStatistics::getAgressionFrequency() const{

	if ((float)(m_raises + m_bets + m_calls + m_checks + m_folds) == 0.0)
		return 0.0;

	if ((float)(m_raises + m_bets) == 0.0)
		return 0.0;

	return (float)(m_raises + m_bets) / (float)(m_raises + m_bets + m_calls + m_checks + m_folds) * 100;
}

float FlopStatistics::getContinuationBetFrequency() const{

	if (m_continuationBetsOpportunities == 0)
		return 0;

	return (float)(m_continuationBets) / (float)(m_continuationBetsOpportunities) * 100;
}

void TurnStatistics::reset(){
	
	m_hands = 0;
	m_folds = 0;
	m_checks = 0;
	m_bets = 0;
	m_calls = 0;
	m_raises = 0;
	m_3Bets = 0;
	m_4Bets = 0;
}

float TurnStatistics::getAgressionFrequency() const{

	if ((float)(m_raises + m_bets + m_calls + m_checks + m_folds) == 0.0)
		return 0.0;

	if ((float)(m_raises + m_bets) == 0.0)
		return 0.0;

	return (float)(m_raises + m_bets) / (float)(m_raises + m_bets + m_calls + m_checks + m_folds) * 100;
}

float TurnStatistics::getAgressionFactor() const{

	if (m_raises + m_bets == 0 || m_calls == 0)
		return 0;

	return (float)(m_raises + m_bets) / (float)m_calls;
}
void RiverStatistics::reset(){
	
	m_hands = 0;
	m_folds = 0;
	m_checks = 0;
	m_bets = 0;
	m_calls = 0;
	m_raises = 0;
	m_3Bets = 0;
	m_4Bets = 0;
}
float RiverStatistics::getAgressionFrequency() const{

	if ((float)(m_raises + m_bets + m_calls + m_checks + m_folds) == 0.0)
		return 0.0;

	if ((float)(m_raises + m_bets) == 0.0)
		return 0.0;

	return (float)(m_raises + m_bets) / (float)(m_raises + m_bets + m_calls + m_checks + m_folds) * 100;
}
float RiverStatistics::getAgressionFactor() const{

	if (m_raises + m_bets == 0 || m_calls == 0)
		return 0;

	return (float)(m_raises + m_bets) / (float)m_calls;
}


