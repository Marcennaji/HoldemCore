// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerFsm.h"

#include <core/engine/CardUtilities.h>
#include <core/engine/HandEvaluator.h>

#include <core/engine/HandFsm.h>
#include <core/engine/model/Ranges.h>
#include <core/interfaces/persistence/IHandAuditStore.h>
#include <core/interfaces/persistence/IPlayersStatisticsStore.h>
#include <core/player/range/RangeParser.h>
#include <core/player/strategy/CurrentHandContext.h>
#include <core/services/GlobalServices.h>
#include "Helpers.h"

#include <sstream>

namespace pkt::core::player
{

using namespace std;

PlayerFsm::PlayerFsm(const GameEvents& events, int id, std::string name, int sC, bool aS, int mB)
    : myID(id), myName(name), myCash(sC), myButton(mB), myEvents(events)
{
    myRangeEstimator = std::make_unique<RangeEstimator>(myID);
    myCurrentHandContext = std::make_unique<CurrentHandContext>();
    loadStatistics();

    int i;
    for (i = 0; i < 2; i++)
    {
        myCards[i] = -1;
    }
}

const PlayerPosition PlayerFsm::getPosition() const
{
    return myPosition;
}

int PlayerFsm::getId() const
{
    return myID;
}
CurrentHandActions& PlayerFsm::getCurrentHandActions()
{
    return myCurrentHandActions;
}

void PlayerFsm::setId(int newId)
{
    myID = newId;
}

void PlayerFsm::setName(const std::string& theValue)
{
    myName = theValue;
}
std::string PlayerFsm::getName() const
{
    return myName;
}

void PlayerFsm::setCash(int theValue)
{
    myCash = theValue;
}
int PlayerFsm::getCash() const
{
    return myCash;
}

void PlayerFsm::setSet(int theValue)
{
    myLastRelativeSet = theValue;
    mySet += theValue;
    myCash -= theValue;
}

void PlayerFsm::setSetNull()
{
    mySet = 0;
    myLastRelativeSet = 0;
}
int PlayerFsm::getSet() const
{
    return mySet;
}
int PlayerFsm::getLastRelativeSet() const
{
    return myLastRelativeSet;
}

void PlayerFsm::setAction(ActionType theValue, bool blind)
{
    myAction = theValue;
}
ActionType PlayerFsm::getAction() const
{
    return myAction;
}

void PlayerFsm::setButton(int theValue)
{
    myButton = theValue;
}
int PlayerFsm::getButton() const
{
    return myButton;
}

void PlayerFsm::setCards(int* theValue)
{

    for (int i = 0; i < 2; i++)
    {
        myCards[i] = theValue[i];
    }

    // will contain human-readable string, i.e "Qc" or "Ts"
    myCard1 = CardUtilities::getCardString(myCards[0]);
    myCard2 = CardUtilities::getCardString(myCards[1]);
}
void PlayerFsm::getCards(int* theValue) const
{
    int i;
    for (i = 0; i < 2; i++)
    {
        theValue[i] = myCards[i];
    }
}

void PlayerFsm::setTurn(bool theValue)
{
    myTurn = theValue;
}
bool PlayerFsm::getTurn() const
{
    return myTurn;
}

void PlayerFsm::setCardsFlip(bool theValue)
{
    myCardsFlip = theValue;
}
bool PlayerFsm::getCardsFlip() const
{
    return myCardsFlip;
}

void PlayerFsm::setHandRanking(int theValue)
{
    myHandRanking = theValue;
}
int PlayerFsm::getHandRanking() const
{
    return myHandRanking;
}

void PlayerFsm::setCashAtHandStart(int theValue)
{
    myCashAtHandStart = theValue;
}
int PlayerFsm::getCashAtHandStart() const
{
    return myCashAtHandStart;
}

void PlayerFsm::setLastMoneyWon(int theValue)
{
    lastMoneyWon = theValue;
}
int PlayerFsm::getLastMoneyWon() const
{
    return lastMoneyWon;
}

std::string PlayerFsm::getCardsValueString() const
{
    std::string s = myCard1 + " " + myCard2;
    return s;
}

const PlayerStatistics& PlayerFsm::getStatistics(const int nbPlayers) const
{
    return myStatistics[nbPlayers];
}
void PlayerFsm::resetPlayerStatistics()
{

    for (int i = 0; i <= MAX_NUMBER_OF_PLAYERS; i++)
    {
        myStatistics[i].reset();
    }
}
/*
void PlayerFsm::updatePreflopStatistics(const int nbPlayers)
{

    if (myCurrentHandActions.getPreflopActions().size() == 1)
    {
        myStatistics[nbPlayers].m_toTalHands++;
        myStatistics[nbPlayers].m_preflopStatistics.m_hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].m_preflopStatistics.m_raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].m_preflopStatistics.m_folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].m_preflopStatistics.m_checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].m_preflopStatistics.m_calls++;
        break;
    default:
        break;
    }

    myStatistics[nbPlayers].m_preflopStatistics.addLastAction(myAction); // keep track of the last 10 actions

    if (myAction == ActionType::Call && currentHand.getRaisersPositions().size() == 0)
    { //
        myStatistics[nbPlayers].m_preflopStatistics.m_limps++;
    }

    int playerRaises = 0;
    for (std::vector<ActionType>::const_iterator i = myCurrentHandActions.getPreflopActions().begin();
         i != myCurrentHandActions.getPreflopActions().end(); i++)
    {
        if (*i == ActionType::Raise || *i == ActionType::Allin)
        {
            playerRaises++;
        }
    }

    if (myAction == ActionType::Raise || myAction == ActionType::Allin)
    {

        if (playerRaises == 1 && currentHand.getRaisersPositions().size() == 2)
        {
            myStatistics[nbPlayers].m_preflopStatistics.m_3Bets++;
        }

        if (playerRaises == 2 || (playerRaises == 1 && currentHand.getRaisersPositions().size() == 3))
        {
            myStatistics[nbPlayers].m_preflopStatistics.m_4Bets++;
        }
    }
    else
    {

        if (playerRaises == 1)
        {

            myStatistics[nbPlayers].m_preflopStatistics.m_call3BetsOpportunities++;

            if (myAction == ActionType::Call)
            {
                myStatistics[nbPlayers].m_preflopStatistics.m_call3Bets++;
            }
        }
    }
}
void PlayerFsm::updateFlopStatistics(const int nbPlayers)
{

    if (myCurrentHandActions.getFlopActions().size() == 1)
    {
        myStatistics[nbPlayers].m_flopStatistics.m_hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].m_flopStatistics.m_raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].m_flopStatistics.m_folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].m_flopStatistics.m_checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].m_flopStatistics.m_calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].m_flopStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand.getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].m_flopStatistics.m_3Bets++;
    }

    // continuation bets
    if (currentHand.getPreflopLastRaiserId() == myID)
    {
        myStatistics[nbPlayers].m_flopStatistics.m_continuationBetsOpportunities++;
        if (myAction == ActionType::Bet)
        {
            myStatistics[nbPlayers].m_flopStatistics.m_continuationBets++;
        }
    }
}
void PlayerFsm::updateTurnStatistics(const int nbPlayers)
{

    if (myCurrentHandActions.getTurnActions().size() == 1)
    {
        myStatistics[nbPlayers].m_turnStatistics.m_hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].m_turnStatistics.m_raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].m_turnStatistics.m_folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].m_turnStatistics.m_checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].m_turnStatistics.m_calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].m_turnStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand.getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].m_turnStatistics.m_3Bets++;
    }
}
void PlayerFsm::updateRiverStatistics(const int nbPlayers)
{

    if (myCurrentHandActions.getRiverActions().size() == 1)
    {
        myStatistics[nbPlayers].m_riverStatistics.m_hands++;
    }

    switch (myAction)
    {
    case ActionType::Allin:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case ActionType::Raise:
        myStatistics[nbPlayers].m_riverStatistics.m_raises++;
        break;
    case ActionType::Fold:
        myStatistics[nbPlayers].m_riverStatistics.m_folds++;
        break;
    case ActionType::Check:
        myStatistics[nbPlayers].m_riverStatistics.m_checks++;
        break;
    case ActionType::Call:
        myStatistics[nbPlayers].m_riverStatistics.m_calls++;
        break;
    case ActionType::Bet:
        myStatistics[nbPlayers].m_riverStatistics.m_bets++;
        break;
    default:
        break;
    }
    if (myAction == ActionType::Raise && currentHand.getRaisersPositions().size() > 1)
    {
        myStatistics[nbPlayers].m_riverStatistics.m_3Bets++;
    }
}
    */

void PlayerFsm::loadStatistics()
{

    resetPlayerStatistics(); // reset stats to 0

    myStatistics = GlobalServices::instance().playersStatisticsStore()->getPlayerStatistics(myName);
    if (myStatistics.empty())
    {
        myStatistics.fill(PlayerStatistics());
    }
}

const PostFlopAnalysisFlags PlayerFsm::getPostFlopAnalysisFlags() const
{
#if (False)
    std::string stringHand = getCardsValueString();
    std::string stringBoard = getStringBoard();

    return GlobalServices::instance().handEvaluationEngine()->analyzeHand(getCardsValueString(), getStringBoard());
#else
    PostFlopAnalysisFlags flags;
    return flags;
#endif
}
bool PlayerFsm::checkIfINeedToShowCards() const
{
#if (False)
    std::list<unsigned> playerNeedToShowCardsList = currentHand.getBoard()->getPlayerNeedToShowCards();
    for (std::list<unsigned>::iterator it = playerNeedToShowCardsList.begin(); it != playerNeedToShowCardsList.end();
         ++it)
    {
        if (*it == myID)
        {
            return true;
        }
    }
#endif

    return false;
}

bool PlayerFsm::getHavePosition(PlayerPosition myPos, PlayerFsmList runningPlayers)
{
    // return true if myPos is last to play, false if not

    bool havePosition = true;

    PlayerFsmListConstIterator itC;

    for (itC = runningPlayers->begin(); itC != runningPlayers->end(); ++itC)
    {

        if ((*itC)->getPosition() > myPos)
        {
            havePosition = false;
        }
    }

    return havePosition;
}

float PlayerFsm::getM() const
{
#if (False)
    int blinds = currentHand.getSmallBlind() + (currentHand.getSmallBlind() * 2); // assume for now that BB is 2 * SB
    if (blinds > 0 && myCash > 0)
    {
        return (float) myCash / blinds;
    }
    else
    {
        return 0;
    }
#else
    return 50;
#endif
}

const HandSimulationStats PlayerFsm::computeHandSimulation() const
{
#if (False)
    const int nbOpponents = mySeatsList->size() - 1;
    // evaluate my strength against my opponents's guessed ranges :
    float maxOpponentsStrengths = getMaxOpponentsStrengths();
    return GlobalServices::instance().handEvaluationEngine()->simulateHandEquity(
        getCardsValueString(), getStringBoard(), nbOpponents, maxOpponentsStrengths);
#else
    return GlobalServices::instance().handEvaluationEngine()->simulateHandEquity("As 6d", "", 2, 0.5);
#endif
}

float PlayerFsm::getMaxOpponentsStrengths() const
{

    std::map<int, float> strenghts = evaluateOpponentsStrengths(); // id player --> % of possible hands that beat us

    float maxOpponentsStrengths = 0;
    int opponentID = -1;

    for (std::map<int, float>::const_iterator i = strenghts.begin(); i != strenghts.end(); i++)
    {

        assert(i->second <= 1.0);
        if (i->second > maxOpponentsStrengths)
        {
            maxOpponentsStrengths = i->second;
        }

        opponentID = i->first;
    }

    //	// if we are facing a single opponent in very loose mode, don't adjust the strength
    //	if (strenghts.size() == 1){
    //		std::shared_ptr<Player> opponent = getPlayerByUniqueId(opponentID);
    //
    //		if (opponent->isInVeryLooseMode(nbPlayers))
    //			return maxOpponentsStrengths;
    //	}
    //
    //	// adjust roughly maxOpponentsStrengths value according to the number of opponents : the more opponents, the
    // more chances we are beaten
    //
    //	const float originMaxOpponentsStrengths = maxOpponentsStrengths;
    //
    //	if (strenghts.size() == 2) // 2 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.1;
    //	else
    //	if (strenghts.size() == 3) // 3 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.2;
    //	else
    //	if (strenghts.size() == 4) // 4 opponents
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.3;
    //	else
    //	if (strenghts.size() > 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.4;
    //
    //	// adjust roughly maxOpponentsStrengths value according to the number of raises : the more raises, the more
    // chances we are beaten
    //
    //	const int nbRaises =	currentHand.getFlopBetsOrRaisesNumber() +
    //							currentHand.getTurnBetsOrRaisesNumber() +
    //							currentHand.getRiverBetsOrRaisesNumber();
    //
    //	if (nbRaises == 2)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.2;
    //	else
    //	if (nbRaises == 3)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 1.5;
    //	else
    //	if (nbRaises == 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 2;
    //	else
    //	if (nbRaises > 4)
    //		maxOpponentsStrengths = maxOpponentsStrengths * 3;
    //
    //	if (maxOpponentsStrengths > 1){

    //		maxOpponentsStrengths = 1;
    //	}

    return maxOpponentsStrengths;
}

// get an opponent winning hands % against me, giving his supposed range
float PlayerFsm::getOpponentWinningHandsPercentage(const int opponentId, std::string board) const
{
#if (False)
    float result = 0;

    auto opponent = *getPlayerFsmListIteratorById(mySeatsList, opponentId);

    assert(myHandRanking > 0);

    // compute winning hands % against my rank
    int nbWinningHands = 0;

    if (opponent->myRangeEstimator->getEstimatedRange().size() == 0)
    {
        opponent->getRangeEstimator()->computeEstimatedPreflopRange(*myCurrentHandContext);
    }

    vector<std::string> ranges = RangeParser::getRangeAtomicValues(opponent->myRangeEstimator->getEstimatedRange());

    vector<std::string> newRanges;

    for (vector<std::string>::const_iterator i = ranges.begin(); i != ranges.end(); i++)
    {

        if ((*i).size() != 4)
        {
            GlobalServices::instance().logger()->error("invalid hand : " + (*i));
            continue;
        }
        string s1 = (*i).substr(0, 2);
        string s2 = (*i).substr(2, 4);

        // delete hands that can't exist, given the board
        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
        {
            continue;
        }

        // delete hands that can't exist, given the player's cards (if they are supposed to be known)
        if (opponentId != myID)
        {
            if (myCard1.find(s1) != string::npos || myCard2.find(s1) != string::npos ||
                myCard1.find(s2) != string::npos || myCard2.find(s2) != string::npos)
            {
                continue;
            }
        }

        newRanges.push_back(*i);
    }
    if (newRanges.size() == 0)
    {
        newRanges.push_back(ANY_CARDS_RANGE);
    }

    for (vector<std::string>::const_iterator i = newRanges.begin(); i != newRanges.end(); i++)
    {
        const int rank = GlobalServices::instance().handEvaluationEngine()->rankHand(((*i) + board).c_str());
        if (rank > myHandRanking)
        {
            nbWinningHands++;
        }
    }
    if (ranges.size() == 0)
    {
        GlobalServices::instance().logger()->error("no ranges for opponent " + std::to_string(opponentId));
        return 0;
    }
    assert(nbWinningHands / ranges.size() <= 1.0);
    return (float) nbWinningHands / (float) newRanges.size();
#else
    return 0.5;
#endif
}

std::map<int, float> PlayerFsm::evaluateOpponentsStrengths() const
{

    map<int, float> result;
#if (False)
    PlayerFsmList players = mySeatsList;
    const int nbPlayers = mySeatsList->size();

    for (PlayerFsmListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID || (*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::None)
        {
            continue;
        }

        const float estimatedOpponentWinningHands = getOpponentWinningHandsPercentage((*it)->getId(), getStringBoard());
        assert(estimatedOpponentWinningHands <= 1.0);

        result[(*it)->getId()] = estimatedOpponentWinningHands;
    }
#endif

    return result;
}

bool PlayerFsm::isPreflopBigBet() const
{
#if (False)
    if (getPotOdd() > 70)
    {
        return true;
    }

    const int highestSet = min(myCash, currentHand.getCurrentBettingRound()->getHighestSet());

    if (highestSet > currentHand.getSmallBlind() * 8 && highestSet - mySet > mySet * 6)
    {
        return true;
    }
#endif
    return false;
}

bool PlayerFsm::isAgressor(const GameState gameState) const
{
#if (False)
    if (gameState == GameStatePreflop && currentHand.getPreflopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == GameStateFlop && currentHand.getFlopLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == GameStateTurn && currentHand.getTurnLastRaiserId() == myID)
    {
        return true;
    }

    if (gameState == GameStateRiver && currentHand.getLastRaiserId() == myID)
    {
        return true;
    }
#endif
    return false;
}
bool PlayerFsm::canBluff(const GameState gameState) const
{

    // check if there is no calling station at the table
    // check also if my opponents stacks are big enough to bluff them
#if (False)
    const int nbPlayers = mySeatsList->size();
    const int nbRaises = currentHand.getPreflopRaisesNumber();

    PlayerFsmList players = currentHand.getRunningPlayersList();

    if (players->size() == 1)
    {
        // all other players are allin
        return false;
    }

    for (PlayerFsmListIterator it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getId() == myID)
        {
            continue;
        }

        PreflopStatistics preflopStats = (*it)->getStatistics(nbPlayers).getPreflopStatistics();

        // if not enough hands, then try to use the statistics collected for (nbPlayers + 1), they should be more
        // accurate
        if (preflopStats.m_hands < MIN_HANDS_STATISTICS_ACCURATE && nbPlayers < 10 &&
            (*it)->getStatistics(nbPlayers + 1).getPreflopStatistics().m_hands > MIN_HANDS_STATISTICS_ACCURATE)
        {

            preflopStats = (*it)->getStatistics(nbPlayers + 1).getPreflopStatistics();
        }

        if ((*it)->getStatistics(nbPlayers).getWentToShowDown() >= 40 &&
            preflopStats.getVoluntaryPutMoneyInPot() - preflopStats.getPreflopRaise() > 15 &&
            preflopStats.getVoluntaryPutMoneyInPot() > 20)
        {
            return false; // seems to be a calling station
        }

        if ((*it)->getCash() < currentHand.getBoard()->getPot() * 3)
        {
            return false;
        }

        if (gameState == GameStatePreflop)
        {
            if (preflopStats.getPreflopCall3BetsFrequency() > 40)
            {
                return false;
            }
        }
    }
#endif
    return true;
}

int PlayerFsm::getPreflopPotOdd() const
{
    return myPreflopPotOdd;
}

void PlayerFsm::setPreflopPotOdd(const int potOdd)
{
    myPreflopPotOdd = potOdd;
}

bool PlayerFsm::isInVeryLooseMode(const int nbPlayers) const
{
    PlayerStatistics stats = getStatistics(nbPlayers);

    PreflopStatistics preflop = stats.getPreflopStatistics();

    if (preflop.getLastActionsNumber(ActionType::Allin) + preflop.getLastActionsNumber(ActionType::Raise) +
            preflop.getLastActionsNumber(ActionType::Call) >
        PreflopStatistics::LAST_ACTIONS_STACK_SIZE * 0.8)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PlayerFsm::updateCurrentHandContext(const GameState state, HandFsm& currentHand)
{
    for (std::vector<ActionType>::const_iterator i = myCurrentHandActions.getFlopActions().begin();
         i != myCurrentHandActions.getFlopActions().end(); i++)
    {

        if (*i == ActionType::Raise)
        {
            myCurrentHandContext->perPlayerContext.nbRaises++;
        }
        else if (*i == ActionType::Bet)
        {
            myCurrentHandContext->perPlayerContext.nbBets++;
        }
        else if (*i == ActionType::Check)
        {
            myCurrentHandContext->perPlayerContext.nbChecks++;
        }
        else if (*i == ActionType::Call)
        {
            myCurrentHandContext->perPlayerContext.nbCalls++;
        }
        else if (*i == ActionType::Allin)
        {
            myCurrentHandContext->perPlayerContext.nbAllins++;
        }
    }

    // Player-specific, visible from the opponents :
    myCurrentHandContext->perPlayerContext.myCash = myCash;
    myCurrentHandContext->perPlayerContext.mySet = mySet;
    myCurrentHandContext->perPlayerContext.myM = static_cast<int>(getM());
    myCurrentHandContext->perPlayerContext.myCurrentHandActions = myCurrentHandActions;
    myCurrentHandContext->perPlayerContext.myHavePosition =
        PlayerFsm::getHavePosition(myPosition, currentHand.getRunningPlayersList());
    myCurrentHandContext->perPlayerContext.myPreflopIsAggressor = isAgressor(GameStatePreflop);
    myCurrentHandContext->perPlayerContext.myFlopIsAggressor = isAgressor(GameStateFlop);
    myCurrentHandContext->perPlayerContext.myTurnIsAggressor = isAgressor(GameStateTurn);
    myCurrentHandContext->perPlayerContext.myRiverIsAggressor = isAgressor(GameStateRiver);
    // myCurrentHandContext->perPlayerContext.myStatistics = getStatistics(mySeatsList->size());
    myCurrentHandContext->perPlayerContext.myID = myID;
    // myCurrentHandContext->perPlayerContext.myIsInVeryLooseMode = isInVeryLooseMode(mySeatsList->size());
    myCurrentHandContext->perPlayerContext.myPosition = myPosition;

    // player specific, hidden from the opponents :
    myCurrentHandContext->perPlayerContext.myCard1 = myCard1;
    myCurrentHandContext->perPlayerContext.myCard2 = myCard2;
    myCurrentHandContext->perPlayerContext.myCanBluff = canBluff(state);
    myCurrentHandContext->perPlayerContext.myPreflopCallingRange = calculatePreflopCallingRange(*myCurrentHandContext);
    // guess what the opponents might have and evaluate our strength, given our hole cards and the board cards (if any)
    myCurrentHandContext->perPlayerContext.myHandSimulation = computeHandSimulation();
    myCurrentHandContext->perPlayerContext.myPostFlopAnalysisFlags = getPostFlopAnalysisFlags();
}

float PlayerFsm::calculatePreflopCallingRange(CurrentHandContext& ctx) const
{
#if (False)
    return myRangeEstimator->getStandardCallingRange(mySeatsList->size());
#else
    return 50;
#endif
}

void PlayerFsm::resetForNewHand()
{
    setCardsFlip(0);
    getCurrentHandActions().reset();
}
void PlayerFsm::setButton(Button b)
{
    myButton = b;
}

} // namespace pkt::core::player
