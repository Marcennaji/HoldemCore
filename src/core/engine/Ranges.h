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

#ifndef RANGES_H
#define RANGES_H

#include <string>

static  const std::string PAIRS = ",22+,";
static  const std::string MEDIUM_PAIRS = ",77,88,99,TT,JJ,";
static  const std::string LOW_PAIRS = ",22,33,44,55,66,";
static  const std::string HIGH_PAIRS = ",QQ+,";

// aces
static  const std::string ACES = ",A2s+,A2o+,";
static  const std::string SUITED_ACES = ",A2s+,";
static  const std::string OFFSUITED_ACES = ",A2o+,";
static  const std::string HIGH_SUITED_ACES = ",ATs+,";
static  const std::string HIGH_OFFSUITED_ACES = ",ATo+,";
static  const std::string LOW_OFFSUITED_ACES = ",A8o,A7o,A6o,A5o,A4o,A3o,A2o,";
static  const std::string LOW_SUITED_ACES = ",A8s,A7s,A6s,A5s,A4s,A3s,A2s,";

// broadways
static  const std::string SUITED_BROADWAYS = ",KQs,KJs,KTs,QJs,QTs,JTs,";
static  const std::string OFFSUITED_BROADWAYS = ",KQo,KJo,KTo,QJo,QTo,JTo,";
static  const std::string BROADWAYS = SUITED_BROADWAYS + OFFSUITED_BROADWAYS;

// connectors
static  const std::string SUITED_CONNECTORS = ",JTs,T9s,98s,87s,76s,65s,54s,";
static  const std::string HIGH_SUITED_CONNECTORS = ",JTs,T9s,98s,";
static  const std::string OFFSUITED_CONNECTORS = ",JTo,T9o,98o,87o,76o,65o,54o,";
static  const std::string HIGH_OFFSUITED_CONNECTORS = ",JTo,T9o,98o,";
static  const std::string CONNECTORS = SUITED_CONNECTORS + OFFSUITED_CONNECTORS;
static  const std::string HIGH_CONNECTORS = HIGH_SUITED_CONNECTORS + HIGH_OFFSUITED_CONNECTORS;

// one gaped
static  const std::string SUITED_ONE_GAPED = ",J9s,T8s,97s,86s,75s,";
static  const std::string OFFSUITED_ONE_GAPED = ",J9o,T8o,97o,86o,75o,";
static  const std::string ONE_GAPED = SUITED_ONE_GAPED + OFFSUITED_ONE_GAPED;

// two-gaped
static  const std::string SUITED_TWO_GAPED = ",Q9s,J8s,T7s,96s,85s,";


// corresponding ranges %
static const float PAIRS_RANGE_VALUE = 5.9;
static const float MEDIUM_PAIRS_RANGE_VALUE = 2.3;
static const float HIGH_PAIRS_RANGE_VALUE = 1.4;
static const float LOW_PAIRS_RANGE_VALUE = 2.3;
static const float ACES_RANGE_VALUE = 14.9;

static const float SUITED_ACES_RANGE_VALUE = 4.1;
static const float OFFSUITED_ACES_RANGE_VALUE = 11.3;
static const float HIGH_SUITED_ACES_RANGE_VALUE = 1.7;
static const float HIGH_OFFSUITED_ACES_RANGE_VALUE = 4.1;
static  const float LOW_OFFSUITED_ACES_RANGE_VALUE = 6.3;
static  const float LOW_SUITED_ACES_RANGE_VALUE = 2.1;

static const float BROADWAYS_RANGE_VALUE = 7.2;
static const float SUITED_BROADWAYS_RANGE_VALUE = 1.8;
static const float OFFSUITED_BROADWAYS_RANGE_VALUE = 5.4;

static const float CONNECTORS_RANGE_VALUE = 8.4;
static const float SUITED_CONNECTORS_RANGE_VALUE = 2.1;
static const float OFFSUITED_CONNECTORS_RANGE_VALUE = 6.3;
static const float HIGH_SUITED_CONNECTORS_RANGE_VALUE = 0.9;
static const float HIGH_OFFSUITED_CONNECTORS_RANGE_VALUE = 2.7;
static const float HIGH_CONNECTORS_RANGE_VALUE = 3.6;

static const float ONE_GAPED_RANGE_VALUE = 6;
static const float SUITED_ONE_GAPED_RANGE_VALUE = 1.5;
static const float OFFSUITED_ONE_GAPED_RANGE_VALUE = 4.5;

static const float SUITED_TWO_GAPED_RANGE_VALUE = 1.5;

// top ranges : the index corresponds to the best % range

// 2-ways equity (heads-up), values taken from HoldemViewer
static const char * TOP_RANGE_2_PLAYERS[] = { 
	",AA,",
	",AA,KK,", // 1%
	",JJ+,",
	",88+,", 
	",77+,AKs,", 
	",77+,AJs+,AKo,", // 5%
	",77+,ATs+,AKo,",
	",77+,ATs+,AQo+,",
	",77+,ATs+,KQs,AJo+,",
	",66+,A9s+,KQs,AJo+,",
	",66+,A8s+,KJs+,ATo+,",// 10
	",66+,A8s+,KTs+,ATo+,KQo,",
	",66+,A7s+,KTs+,ATo+,KQo,",
	",66+,A7s+,KTs+,A9o+,KJo+,",
	",55+,A7s+,KTs+,A9o+,KJo+,",
	",55+,A5s+,K9s+,QJs,A9o+,KJo+,", // 15 %
	",55+,A5s+,K9s+,QJs,A8o+,KJo+,",
	",55+,A5s+,K9s+,QJs,A8o+,KTo+,",
	",55+,A4s+,K9s+,QTs+,A7o+,KTo+,",
	",55+,A3s+,K8s+,QTs+,A7o+,KTo+,",
	",55+,A3s+,K8s+,QTs+,A7o+,KTo+,QJo,", // 20
	",55+,A3s+,K8s+,QTs+,A7o+,K9o+,QJo,",
	",55+,A3s+,K8s+,QTs+,A7o+,A5o,K9o+,QJo,",
	",55+,A3s+,K8s+,Q9s+,A5o+,K9o+,QJo,",
	",55+,A2s+,K7s+,Q9s+,JTs,A5o+,K9o+,QJo,",
	",44+,A2s+,K7s+,Q9s+,JTs,A5o+,K9o+,QTo+,", // 25
	",44+,A2s+,K7s+,Q9s+,JTs,A4o+,K9o+,QTo+,",
	",44+,A2s+,K6s+,Q9s+,JTs,A4o+,K8o+,QTo+,",
	",44+,A2s+,K6s+,Q8s+,JTs,A4o+,K8o+,QTo+,",
	",44+,A2s+,K5s+,Q8s+,J9s+,A3o+,K8o+,QTo+,",
	",44+,A2s+,K5s+,Q8s+,J9s+,A3o+,K8o+,Q9o+,", // 30
	",44+,A2s+,K5s+,Q8s+,J9s+,A3o+,K8o+,Q9o+,JTo,",
	",44+,A2s+,K5s+,Q8s+,J9s+,A3o+,K7o+,Q9o+,JTo,",
	",44+,A2s+,K4s+,Q8s+,J9s+,A2o+,K7o+,Q9o+,JTo,",
	",44+,A2s+,K4s+,Q7s+,J9s+,A2o+,K6o+,Q9o+,JTo,",
	",44+,A2s+,K3s+,Q7s+,J8s+,T9s,A2o+,K6o+,Q9o+,JTo,", // 35
	",33+,A2s+,K3s+,Q6s+,J8s+,T9s,A2o+,K6o+,Q9o+,JTo,",
	",33+,A2s+,K3s+,Q6s+,J8s+,T9s,A2o+,K6o+,Q8o+,JTo,",
	",33+,A2s+,K3s+,Q6s+,J8s+,T9s,A2o+,K5o+,Q8o+,JTo,",
	",33+,A2s+,K2s+,Q6s+,J8s+,T9s,A2o+,K5o+,Q8o+,J9o+,",
	",33+,A2s+,K2s+,Q5s+,J8s+,T8s+,A2o+,K4o+,Q8o+,J9o+,", // 40
	",33+,A2s+,K2s+,Q4s+,J7s+,T8s+,A2o+,K4o+,Q8o+,J9o+,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T8s+,A2o+,K4o+,Q7o+,J9o+,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T8s+,A2o+,K4o+,Q7o+,J9o+,T9o,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T8s+,A2o+,K4o+,Q7o+,J8o+,T9o,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T8s+,A2o+,K3o+,Q7o+,J8o+,T9o,", // 45
	",33+,A2s+,K2s+,Q3s+,J7s+,T8s+,A2o+,K3o+,Q6o+,J8o+,T9o,",
	",33+,A2s+,K2s+,Q3s+,J6s+,T7s+,98s,A2o+,K3o+,Q6o+,J8o+,T9o,",
	",22+,A2s+,K2s+,Q3s+,J6s+,T7s+,98s,A2o+,K2o+,Q6o+,J8o+,T9o,",
	",22+,A2s+,K2s+,Q2s+,J6s+,T7s+,98s,A2o+,K2o+,Q5o+,J8o+,T9o,",
	",22+,A2s+,K2s+,Q2s+,J5s+,T7s+,98s,A2o+,K2o+,Q5o+,J8o+,T9o,", // 50
	",22+,A2s+,K2s+,Q2s+,J5s+,T7s+,98s,A2o+,K2o+,Q5o+,J7o+,T8o+,",
	",22+,A2s+,K2s+,Q2s+,J5s+,T7s+,98s,A2o+,K2o+,Q4o+,J7o+,T8o+,",
	",22+,A2s+,K2s+,Q2s+,J4s+,T7s+,97s+,A2o+,K2o+,Q4o+,J7o+,T8o+,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,97s+,A2o+,K2o+,Q3o+,J7o+,T8o+,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,97s+,A2o+,K2o+,Q3o+,J7o+,T8o+,98o,", // 55
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,97s+,87s,A2o+,K2o+,Q3o+,J7o+,T8o+,98o,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,97s+,87s,A2o+,K2o+,Q3o+,J7o+,T7o+,98o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T6s+,96s+,87s,A2o+,K2o+,Q3o+,J6o+,T7o+,98o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T6s+,96s+,87s,A2o+,K2o+,Q2o+,J6o+,T7o+,98o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T5s+,96s+,87s,A2o+,K2o+,Q2o+,J5o+,T7o+,98o,", // 60
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,96s+,87s,A2o+,K2o+,Q2o+,J5o+,T7o+,97o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,96s+,86s+,A2o+,K2o+,Q2o+,J5o+,T7o+,97o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,96s+,86s+,A2o+,K2o+,Q2o+,J4o+,T7o+,97o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,86s+,A2o+,K2o+,Q2o+,J4o+,T6o+,97o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,86s+,76s,A2o+,K2o+,Q2o+,J3o+,T6o+,97o+,", // 65
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,86s+,76s,A2o+,K2o+,Q2o+,J3o+,T6o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,76s,A2o+,K2o+,Q2o+,J3o+,T6o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,76s,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,76s,A2o+,K2o+,Q2o+,J2o+,T6o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,A2o+,K2o+,Q2o+,J2o+,T5o+,96o+,87o,",// 70
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,A2o+,K2o+,Q2o+,J2o+,T4o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,85s+,75s+,A2o+,K2o+,Q2o+,J2o+,T4o+,96o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,75s+,65s,A2o+,K2o+,Q2o+,J2o+,T4o+,96o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,75s+,65s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,75s+,65s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,86o+,",// 75
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,75s+,65s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,65s,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,65s,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,83s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,83s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,76o,",//80
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,75o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,64s+,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,85o+,75o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,64s+,54s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,75o+,65o,",//85
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,63s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,83o+,74o+,64o+,54o,",//90
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,83o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,64o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,63o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,63o+,53o+,",//95
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,63o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,32o,"// 100

};

// 3-ways equity, values taken from HoldemViewer
static const char * TOP_RANGE_3_PLAYERS[] = { 
	",AA,",
	",AA,KK,",// 1%
	",JJ+,",
	",99+,AKs,", 
	",88+,AQs+,", 
	",88+,AJs+,AKo,",// 5 %
	",88+,ATs+,KQs,AKo,",
	",77+,ATs+,KQs,AQo+,",
	",77+,ATs+,KJs+,AJo+,",
	",77+,A9s+,KTs+,AJo+,",
	",77+,A9s+,KTs+,AJo+,KQo,",// 10 %
	",77+,A9s+,KTs+,QJs,ATo+,KQo,",
	",66+,A8s+,KTs+,QTs+,ATo+,KQo,",
	",66+,A8s+,K9s+,QTs+,ATo+,KJo+,",
	",66+,A7s+,K9s+,QTs+,JTs,ATo+,KJo+,",
	",66+,A7s+,K9s+,QTs+,JTs,ATo+,KTo+,",// 15 %
	",66+,A7s+,K9s+,QTs+,JTs,A9o+,KTo+,QJo,",
	",66+,A5s+,K9s+,QTs+,JTs,A9o+,KTo+,QJo,",
	",55+,A4s+,K9s+,Q9s+,JTs,A9o+,KTo+,QJo,",
	",55+,A4s+,K9s+,Q9s+,JTs,A8o+,KTo+,QJo,",
	",55+,A4s+,K9s+,Q9s+,JTs,A8o+,KTo+,QTo+,",// 20 %
	",55+,A3s+,K8s+,Q9s+,J9s+,A8o+,KTo+,QTo+,",
	",55+,A3s+,K7s+,Q9s+,J9s+,A8o+,K9o+,QTo+,",
	",55+,A3s+,K7s+,Q9s+,J9s+,A7o+,K9o+,QTo+,",
	",55+,A3s+,K7s+,Q9s+,J9s+,T9s,A7o+,K9o+,QTo+,JTo,",
	",55+,A2s+,K6s+,Q8s+,J9s+,T9s,A7o+,K9o+,QTo+,JTo,",//25
	",55+,A2s+,K6s+,Q8s+,J9s+,T9s,A7o+,A5o,K9o+,QTo+,JTo,",
	",55+,A2s+,K6s+,Q8s+,J9s+,T9s,A5o+,K9o+,QTo+,JTo,",
	",55+,A2s+,K6s+,Q8s+,J8s+,T9s,A5o+,K9o+,Q9o+,JTo,",
	",44+,A2s+,K5s+,Q8s+,J8s+,T9s,A5o+,K9o+,Q9o+,JTo,",
	",44+,A2s+,K5s+,Q8s+,J8s+,T9s,A4o+,K9o+,Q9o+,JTo,",//30
	",44+,A2s+,K5s+,Q8s+,J8s+,T8s+,A4o+,K8o+,Q9o+,JTo,",
	",44+,A2s+,K4s+,Q7s+,J8s+,T8s+,A4o+,K8o+,Q9o+,J9o+,",
	",44+,A2s+,K4s+,Q7s+,J8s+,T8s+,A3o+,K8o+,Q9o+,J9o+,",
	",44+,A2s+,K4s+,Q7s+,J8s+,T8s+,98s,A3o+,K8o+,Q9o+,J9o+,",
	",44+,A2s+,K3s+,Q6s+,J8s+,T8s+,98s,A3o+,K7o+,Q9o+,J9o+,",//35
	",44+,A2s+,K3s+,Q6s+,J8s+,T8s+,98s,A3o+,K7o+,Q9o+,J9o+,T9o,",
	",44+,A2s+,K3s+,Q6s+,J7s+,T8s+,98s,A3o+,K7o+,Q8o+,J9o+,T9o,",
	",44+,A2s+,K3s+,Q6s+,J7s+,T8s+,98s,A2o+,K7o+,Q8o+,J9o+,T9o,",
	",44+,A2s+,K2s+,Q6s+,J7s+,T8s+,98s,A2o+,K6o+,Q8o+,J9o+,T9o,",
	",44+,A2s+,K2s+,Q5s+,J7s+,T7s+,98s,A2o+,K6o+,Q8o+,J9o+,T9o,",//40
	",44+,A2s+,K2s+,Q5s+,J7s+,T7s+,98s,A2o+,K6o+,Q8o+,J8o+,T9o,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,A2o+,K6o+,Q8o+,J8o+,T9o,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,87s,A2o+,K5o+,Q8o+,J8o+,T9o,",
	",33+,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,87s,A2o+,K5o+,Q8o+,J8o+,T8o+,",
	",33+,A2s+,K2s+,Q3s+,J6s+,T7s+,97s+,87s,A2o+,K5o+,Q8o+,J8o+,T8o+,",//45
	",33+,A2s+,K2s+,Q3s+,J6s+,T7s+,97s+,87s,A2o+,K5o+,Q7o+,J8o+,T8o+,",
	",33+,A2s+,K2s+,Q3s+,J5s+,T6s+,97s+,87s,A2o+,K4o+,Q7o+,J8o+,T8o+,",
	",33+,A2s+,K2s+,Q3s+,J5s+,T6s+,97s+,87s,A2o+,K4o+,Q7o+,J8o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J5s+,T6s+,97s+,87s,A2o+,K4o+,Q6o+,J8o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J5s+,T6s+,96s+,87s,A2o+,K3o+,Q6o+,J8o+,T8o+,98o,",//50
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,A2o+,K3o+,Q6o+,J8o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,A2o+,K3o+,Q6o+,J7o+,T8o+,98o",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,A2o+,K3o+,Q5o+,J7o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,A2o+,K3o+,Q5o+,J7o+,T7o+,98o,", 
	",33+,A2s+,K2s+,Q2s+,J3s+,T6s+,96s+,86s+,76s,A2o+,K2o+,Q5o+,J7o+,T7o+,98o,", // 55
	",22+,A2s+,K2s+,Q2s+,J3s+,T5s+,96s+,86s+,76s,A2o+,K2o+,Q5o+,J7o+,T7o+,98o,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T5s+,96s+,86s+,76s,A2o+,K2o+,Q5o+,J7o+,T7o+,97o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T5s+,96s+,86s+,76s,A2o+,K2o+,Q5o+,J7o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T5s+,96s+,86s+,76s,A2o+,K2o+,Q5o+,J7o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T5s+,96s+,86s+,76s,A2o+,K2o+,Q4o+,J7o+,T7o+,97o+,87o,", // 60
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,95s+,86s+,76s,65s,A2o+,K2o+,Q4o+,J7o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,95s+,85s+,75s+,65s,A2o+,K2o+,Q4o+,J6o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T4s+,95s+,85s+,75s+,65s,A2o+,K2o+,Q3o+,J6o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,85s+,75s+,65s,A2o+,K2o+,Q3o+,J6o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,85s+,75s+,65s,A2o+,K2o+,Q3o+,J5o+,T6o+,97o+,87o,", // 65
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,75s+,65s,54s,A2o+,K2o+,Q3o+,J5o+,T6o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",//70
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J4o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,64s+,53s+,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,65o,",// 75
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,83s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,85o+,75o+,65o,",// 80
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,64o+,54o,",// 85
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,75o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,53o+,",//90
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,64o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,73o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,83o+,73o+,63o+,53o+,43o,",//95
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,63o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,32o,"//100
};

static const char * TOP_RANGE_4_PLAYERS[] = {
	",AA,",
	",AA,KK,",// 1%
	",JJ+,",
	",99+,AKs,", 
	",99+,AQs+,AKo,", 
	",99+,AJs+,KQs,AKo,",// 5 %
	",88+,ATs+,KQs,AKo,",
	",88+,ATs+,KTs+,AQo+,",
	",88+,ATs+,KTs+,QJs,AJo+,",
	",88+,ATs+,KTs+,QJs,AJo+,KQo,",
	",88+,A9s+,KTs+,QTs+,AJo+,KQo,",// 10 %
	",77+,A9s+,KTs+,QTs+,ATo+,KQo,",
	",77+,A9s+,KTs+,QTs+,JTs,ATo+,KJo+,",
	",77+,A8s+,K9s+,QTs+,JTs,ATo+,KJo+,",
	",77+,A8s+,K9s+,QTs+,JTs,ATo+,KJo+,QJo,",
	",77+,A7s+,K9s+,QTs+,JTs,ATo+,KTo+,QJo,",// 15 %
	",66+,A7s+,A5s,K9s+,Q9s+,JTs,ATo+,KTo+,QJo,",
	",66+,A5s+,K9s+,Q9s+,JTs,ATo+,KTo+,QTo+,",
	",66+,A5s+,K9s+,Q9s+,J9s+,ATo+,KTo+,QTo+,",
	",66+,A5s+,K9s+,Q9s+,J9s+,T9s,A9o+,KTo+,QTo+,",
	",66+,A4s+,K8s+,Q9s+,J9s+,T9s,A9o+,KTo+,QTo+,JTo,",// 20 %
	",66+,A4s+,K7s+,Q9s+,J9s+,T9s,A9o+,KTo+,QTo+,JTo,",
	",66+,A3s+,K7s+,Q9s+,J9s+,T9s,A8o+,KTo+,QTo+,JTo,",
	",66+,A3s+,K7s+,Q8s+,J9s+,T9s,A8o+,K9o+,QTo+,JTo,",
	",66+,A2s+,K6s+,Q8s+,J8s+,T9s,A8o+,K9o+,QTo+,JTo,",
	",66+,A2s+,K6s+,Q8s+,J8s+,T8s+,A7o+,K9o+,QTo+,JTo,",//25
	",55+,A2s+,K6s+,Q8s+,J8s+,T8s+,A7o+,K9o+,QTo+,JTo,",
	",55+,A2s+,K5s+,Q8s+,J8s+,T8s+,98s,A7o+,K9o+,Q9o+,JTo,",
	",55+,A2s+,K5s+,Q7s+,J8s+,T8s+,98s,A7o+,K9o+,Q9o+,J9o+,",
	",55+,A2s+,K5s+,Q7s+,J8s+,T8s+,98s,A7o+,A5o,K9o+,Q9o+,J9o+,",
	",55+,A2s+,K5s+,Q7s+,J8s+,T8s+,98s,A7o+,A5o,K9o+,Q9o+,J9o+,T9o,",//30
	",55+,A2s+,K5s+,Q7s+,J8s+,T8s+,98s,A5o+,K9o+,Q9o+,J9o+,T9o,",
	",55+,A2s+,K4s+,Q7s+,J8s+,T8s+,98s,A5o+,K8o+,Q9o+,J9o+,T9o,",
	",55+,A2s+,K4s+,Q6s+,J7s+,T8s+,98s,A5o+,K8o+,Q9o+,J9o+,T9o,",
	",55+,A2s+,K4s+,Q6s+,J7s+,T7s+,98s,A4o+,K8o+,Q9o+,J9o+,T9o,",
	",55+,A2s+,K3s+,Q6s+,J7s+,T7s+,97s+,87s,A4o+,K8o+,Q9o+,J9o+,T9o,",//35
	",55+,A2s+,K3s+,Q5s+,J7s+,T7s+,97s+,87s,A4o+,K7o+,Q9o+,J9o+,T9o,",
	",44+,A2s+,K3s+,Q5s+,J7s+,T7s+,97s+,87s,A4o+,K7o+,Q9o+,J9o+,T9o,",
	",44+,A2s+,K3s+,Q5s+,J7s+,T7s+,97s+,87s,A4o+,K7o+,Q8o+,J9o+,T9o,",
	",44+,A2s+,K2s+,Q5s+,J7s+,T7s+,97s+,87s,A3o+,K7o+,Q8o+,J9o+,T9o,",
	",44+,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,87s,A3o+,K7o+,Q8o+,J8o+,T9o,",//40
	",44+,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,87s,A3o+,K7o+,Q8o+,J8o+,T8o+,",
	",44+,A2s+,K2s+,Q4s+,J6s+,T7s+,97s+,87s,A3o+,K6o+,Q8o+,J8o+,T8o+,",
	",44+,A2s+,K2s+,Q4s+,J6s+,T7s+,97s+,87s,A2o+,K6o+,Q8o+,J8o+,T8o+,",
	",44+,A2s+,K2s+,Q4s+,J6s+,T6s+,97s+,87s,A2o+,K6o+,Q8o+,J8o+,T8o+,98o,",
	",44+,A2s+,K2s+,Q4s+,J6s+,T6s+,96s+,86s+,76s,A2o+,K6o+,Q8o+,J8o+,T8o+,98o,",//45
	",44+,A2s+,K2s+,Q3s+,J5s+,T6s+,96s+,86s+,76s,A2o+,K6o+,Q8o+,J8o+,T8o+,98o,",
	",44+,A2s+,K2s+,Q3s+,J5s+,T6s+,96s+,86s+,76s,A2o+,K5o+,Q8o+,J8o+,T8o+,98o,",
	",44+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,A2o+,K5o+,Q7o+,J8o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,65s,A2o+,K5o+,Q7o+,J8o+,T8o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,65s,A2o+,K5o+,Q7o+,J7o+,T8o+,98o,",//50
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,76s,65s,A2o+,K5o+,Q7o+,J7o+,T7o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,86s+,75s+,65s,A2o+,K4o+,Q7o+,J7o+,T7o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J4s+,T5s+,96s+,86s+,75s+,65s,A2o+,K4o+,Q6o+,J7o+,T7o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J3s+,T5s+,95s+,86s+,75s+,65s,A2o+,K4o+,Q6o+,J7o+,T7o+,98o,",
	",33+,A2s+,K2s+,Q2s+,J3s+,T5s+,95s+,85s+,75s+,65s,A2o+,K4o+,Q6o+,J7o+,T7o+,98o,87o,", // 55
	",33+,A2s+,K2s+,Q2s+,J3s+,T4s+,95s+,85s+,75s+,65s,A2o+,K4o+,Q6o+,J7o+,T7o+,97o+,87o,",
	",33+,A2s+,K2s+,Q2s+,J3s+,T4s+,95s+,85s+,75s+,65s,A2o+,K3o+,Q6o+,J7o+,T7o+,97o+,87o,",
	",33+,A2s+,K2s+,Q2s+,J2s+,T4s+,95s+,85s+,75s+,65s,54s,A2o+,K3o+,Q6o+,J7o+,T7o+,97o+,87o,",
	",33+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,85s+,75s+,64s+,54s,A2o+,K3o+,Q5o+,J7o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,85s+,75s+,64s+,54s,A2o+,K3o+,Q5o+,J7o+,T7o+,97o+,87o,", // 60
	",22+,A2s+,K2s+,Q2s+,J2s+,T3s+,95s+,85s+,74s+,64s+,54s,A2o+,K2o+,Q5o+,J7o+,T7o+,97o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,74s+,64s+,54s,A2o+,K2o+,Q5o+,J7o+,T7o+,97o+,87o,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,74s+,64s+,54s,A2o+,K2o+,Q4o+,J7o+,T7o+,97o+,87o,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q4o+,J6o+,T7o+,97o+,87o,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q4o+,J6o+,T7o+,97o+,86o+,76o,", // 65
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q4o+,J6o+,T6o+,97o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",//70
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J4o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,64s+,53s+,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,65o,",// 75
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,83s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,85o+,75o+,65o,",// 80
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,64o+,54o,",// 85
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,75o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,53o+,",//90
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,64o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,73o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,83o+,73o+,63o+,53o+,43o,",//95
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,63o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,32o,"//100
};

static const char * TOP_RANGE_MORE_4_PLAYERS[] = {
	",AA,",
	",AA,KK,",// 1%
	",JJ+,AKs,",
	",JJ+,AQs+,AKo,", 
	",TT+,AJs+,AKo,", 
	",99+,ATs+,KQs,AKo,",// 5 %
	",99+,ATs+,KQs,AQo+,",
	",99+,ATs+,KJs+,QJs,JTs,AQo+,",
	",88+,ATs+,KJs+,QJs,JTs,AJo+,",
	",88+,ATs+,KTs+,QJs,JTs,AJo+,",
	",88+,ATs+,KTs+,QTs+,J9s+,AJo+,KQo,",// 10 %
	",77+,ATs+,KTs+,QTs+,J9s+,T9s,98s,AJo+,KQo,",
	",77+,A7s+,KTs+,QTs+,J9s+,T9s,98s,AJo+,KQo,",
	",77+,A7s+,KTs+,QTs+,J9s+,T9s,98s,AJo+,KJo+,",
	",77+,A4s+,KTs+,QTs+,J9s+,T9s,98s,AJo+,KJo+,",
	",77+,A3s+,KTs+,QTs+,J9s+,T9s,98s,AJo+,KJo+,QJo,",// 15 %
	",77+,A2s+,KTs+,Q9s+,J9s+,T9s,98s,AJo+,KJo+,QJo,",
	",77+,A2s+,KTs+,Q9s+,J9s+,T8s+,98s,AJo+,KJo+,QJo,JTo,",
	",77+,A2s+,KTs+,Q9s+,J9s+,T8s+,97s+,87s,76s,65s,AJo+,KJo+,QJo,JTo,",
	",66+,A2s+,KTs+,Q9s+,J9s+,T8s+,97s+,87s,76s,65s,AJo+,KJo+,QJo,JTo,",
	",55+,A2s+,KTs+,Q9s+,J9s+,T8s+,97s+,87s,76s,65s,ATo+,KJo+,QJo,JTo,",// 20 %
	",55+,A2s+,K9s+,Q9s+,J9s+,T8s+,97s+,87s,76s,65s,ATo+,KTo+,QJo,JTo,",
	",55+,A2s+,K9s+,Q9s+,J9s+,T8s+,97s+,87s,76s,65s,ATo+,KTo+,QTo+,JTo,",
	",55+,A2s+,K9s+,Q9s+,J8s+,T8s+,97s+,86s+,75s+,65s,ATo+,KTo+,QTo+,JTo,",
	",55+,A2s+,K7s+,Q9s+,J8s+,T8s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,JTo,",
	",44+,A2s+,K6s+,Q8s+,J8s+,T8s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,JTo,",//25
	",44+,A2s+,K3s+,Q8s+,J8s+,T8s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,JTo,",
	",33+,A2s+,K3s+,Q8s+,J8s+,T8s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,J9o+,",
	",33+,A2s+,K2s+,Q8s+,J8s+,T8s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,J9o+,T9o,",
	",22+,A2s+,K2s+,Q8s+,J8s+,T7s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,J9o+,T9o,",
	",22+,A2s+,K2s+,Q8s+,J8s+,T7s+,97s+,86s+,75s+,65s,54s,ATo+,KTo+,QTo+,J9o+,T9o,98o,",//30
	",22+,A2s+,K2s+,Q8s+,J8s+,T7s+,97s+,86s+,75s+,64s+,53s+,43s,ATo+,KTo+,QTo+,J9o+,T9o,98o,",
	",22+,A2s+,K2s+,Q8s+,J8s+,T7s+,97s+,86s+,75s+,64s+,53s+,43s,A9o+,KTo+,QTo+,J9o+,T9o,98o,",
	",22+,A2s+,K2s+,Q8s+,J8s+,T7s+,97s+,86s+,75s+,64s+,53s+,43s,A9o+,K9o+,QTo+,J9o+,T9o,98o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,97s+,86s+,75s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J9o+,T9o,98o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,97s+,86s+,75s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T9o,98o,",//35
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,86s+,75s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,86s+,75s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,75s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,43s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",//40
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A9o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A8o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A7o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A7o+,A5o,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A4o+,K9o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",//45
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A4o+,K8o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A3o+,K8o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A3o+,K7o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q8s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K7o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q7s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K6o+,Q9o+,J8o+,T8o+,98o,87o,76o,65o,54o,",//50
	",22+,A2s+,K2s+,Q6s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K6o+,Q8o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q6s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K5o+,Q8o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q5s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K5o+,Q8o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q4s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K4o+,Q8o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q4s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K4o+,Q7o+,J8o+,T8o+,98o,87o,76o,65o,54o,",// 55
	",22+,A2s+,K2s+,Q4s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K3o+,Q7o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q3s+,J7s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K3o+,Q6o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q3s+,J6s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q6o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J6s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q5o+,J8o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J5s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q5o+,J8o+,T8o+,98o,87o,76o,65o,54o,",// 60
	",22+,A2s+,K2s+,Q2s+,J5s+,T7s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q4o+,J7o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J4s+,T6s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q4o+,J7o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q3o+,J7o+,T8o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q3o+,J7o+,T7o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J3s+,T6s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q3o+,J6o+,T7o+,98o,87o,76o,65o,54o,",// 65
	",22+,A2s+,K2s+,Q2s+,J2s+,T6s+,96s+,85s+,74s+,64s+,53s+,42s+,32s,A2o+,K2o+,Q2o+,J6o+,T7o+,98o,87o,76o,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,95s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,87o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,85s+,75s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,94s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J5o+,T6o+,96o+,86o+,76o,",//70
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J4o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,93s+,84s+,74s+,64s+,54s,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,64s+,53s+,A2o+,K2o+,Q2o+,J3o+,T6o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,96o+,86o+,76o,65o,",// 75
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,A2o+,K2o+,Q2o+,J3o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,74s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,76o,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T5o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,84s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,86o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,83s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T4o+,95o+,85o+,75o+,65o,",// 80
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,53s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,63s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,43s,A2o+,K2o+,Q2o+,J2o+,T3o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,65o,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,73s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,95o+,85o+,75o+,64o+,54o,",// 85
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,75o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,85o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,A2o+,K2o+,Q2o+,J2o+,T2o+,94o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,54o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,93o+,84o+,74o+,64o+,53o+,",//90
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,64o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,74o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,84o+,73o+,63o+,53o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,83o+,73o+,63o+,53o+,43o,",//95
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,63o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,43o,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,73o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,",
	",22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,32o,"//100
};

static const char * ANY_CARDS_RANGE = "22+,A2s+,K2s+,Q2s+,J2s+,T2s+,92s+,82s+,72s+,62s+,52s+,42s+,32s,A2o+,K2o+,Q2o+,J2o+,T2o+,92o+,82o+,72o+,62o+,52o+,42o+,32o";

#endif