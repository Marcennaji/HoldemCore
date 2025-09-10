
#include "Helpers.h"
#include "core/engine/BettingActions.h"
#include "core/engine/CardUtilities.h"
#include "core/engine/Exception.h"
#include "core/engine/HandFsm.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/PlayerFsm.h"
#include "core/player/deprecated/Player.h"
#include "core/services/GlobalServices.h"

// Include FSM states for the helper function
#include "core/engine/round_states/FlopState.h"
#include "core/engine/round_states/PostRiverState.h"
#include "core/engine/round_states/RiverState.h"

#include <algorithm>
#include "core/engine/round_states/TurnState.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <string.h>
#include <string>

namespace pkt::core::player
{
using namespace std;

// values are odd %, according to the outs number. Array index is the number of outs
static int outsOddsOneCard[] = {
    0,  2,  4,  6,  8,  11, /* 0 to 5 outs */
    13, 15, 17, 19, 21,     /* 6 to 10 outs */
    24, 26, 28, 30, 32,     /* 11 to 15 outs */
    34, 36, 39, 41, 43      /* 16 to 20 outs */
};

static int outsOddsTwoCard[] = {
    0,  4,  8,  13, 17, 20, /* 0 to 5 outs */
    24, 28, 32, 35, 38,     /* 6 to 10 outs */
    42, 45, 48, 51, 54,     /* 11 to 15 outs */
    57, 60, 62, 65, 68      /* 16 to 20 outs */
};

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId)
{
    std::vector<std::shared_ptr<Player>> v(players.begin(), players.end());
    auto it = std::find_if(v.begin(), v.end(), [=](auto& p) { return p->getId() == humanId; });
    if (it != v.end())
    {
        std::swap(v.front(), *it);
    }

    std::mt19937 rng(std::time(nullptr));
    std::shuffle(v.begin() + 1, v.end(), rng);

    players.assign(v.begin(), v.end());
}

string getFakeCard(char c)
{

    char tmp[3];
    tmp[0] = c;
    tmp[1] = 'c';
    tmp[2] = NULL;
    return string(tmp);
}

int getBoardCardsHigherThan(std::string stringBoard, std::string card)
{

    std::istringstream oss(stringBoard);
    std::string boardCard;

    int n = 0;

    while (getline(oss, boardCard, ' '))
    {

        if (CardUtilities::getCardValue(boardCard) > CardUtilities::getCardValue(card))
        {
            n++;
        }
    }
    return n;
}

bool isCardsInRange(string card1, string card2, string ranges)
{
    // Ensure the first card is the highest
    ensureHighestCard(card1, card2);

    const char* c1 = card1.c_str();
    const char* c2 = card2.c_str();

    std::istringstream oss(ranges);
    std::string token;

    while (getline(oss, token, ','))
    {
        if (token.empty())
        {
            continue;
        }

        if (!isValidRange(token))
        {
            return false;
        }

        const char* range = token.c_str();

        if (isExactPair(c1, c2, range))
        {
            return true;
        }

        if (isExactSuitedHand(c1, c2, range))
        {
            return true;
        }

        if (isExactOffsuitedHand(c1, c2, range))
        {
            return true;
        }

        if (isPairAndAbove(card1, card2, range))
        {
            return true;
        }

        if (isOffsuitedAndAbove(card1, card2, c1, c2, range))
        {
            return true;
        }

        if (isSuitedAndAbove(card1, card2, c1, c2, range))
        {
            return true;
        }

        if (isExactHand(card1, card2, range))
        {
            return true;
        }
    }

    return false;
}
void ensureHighestCard(string& card1, string& card2)
{
    if (CardUtilities::getCardValue(card1) < CardUtilities::getCardValue(card2))
    {
        std::swap(card1, card2);
    }
}
bool isValidRange(const std::string& token)
{
    return !(token.size() == 1 || token.size() > 4);
}
bool isExactPair(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 2 && c1[0] == range[0] && c2[0] == range[1]);
}
bool isExactSuitedHand(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 3 && range[2] == 's' &&
            ((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) && (c1[1] == c2[1]));
}
bool isExactOffsuitedHand(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 3 && range[2] == 'o' &&
            ((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) && (c1[1] != c2[1]));
}
bool isPairAndAbove(const string& card1, const string& card2, const char* range)
{
    return (strlen(range) == 3 && range[0] == range[1] && range[2] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(card2) &&
            CardUtilities::getCardValue(card1) >= CardUtilities::getCardValue(getFakeCard(range[0])));
}
bool isOffsuitedAndAbove(const string& card1, const string& card2, const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 4 && range[2] == 'o' && range[3] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(getFakeCard(range[0])) &&
            CardUtilities::getCardValue(card2) >= CardUtilities::getCardValue(getFakeCard(range[1])) &&
            CardUtilities::getCardValue(card2) < CardUtilities::getCardValue(card1) && c1[1] != c2[1]);
}
bool isSuitedAndAbove(const string& card1, const string& card2, const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 4 && range[2] == 's' && range[3] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(getFakeCard(range[0])) &&
            CardUtilities::getCardValue(card2) >= CardUtilities::getCardValue(getFakeCard(range[1])) &&
            CardUtilities::getCardValue(card2) < CardUtilities::getCardValue(card1) && c1[1] == c2[1]);
}
bool isExactHand(const string& card1, const string& card2, const char* range)
{
    if (strlen(range) == 4 && range[2] != 's' && range[2] != 'o')
    {
        string exactCard1 = {range[0], range[1]};
        string exactCard2 = {range[2], range[3]};

        return (card1 == exactCard1 && card2 == exactCard2) || (card1 == exactCard2 && card2 == exactCard1);
    }
    return false;
}
// returns a % chance, for a winning draw
const int getDrawingProbability(const PostFlopAnalysisFlags& postFlopAnalysis)
{

    if (!postFlopAnalysis.usesFirst && !postFlopAnalysis.usesSecond)
    {
        return 0;
    }

    int outs = 0;

    // do not count outs for straight or flush, is the board is paired

    if (!postFlopAnalysis.isFullHousePossible)
    {
        outs = postFlopAnalysis.straightOuts + postFlopAnalysis.flushOuts + postFlopAnalysis.betterOuts;
    }
    else
    {
        outs = postFlopAnalysis.betterOuts;
    }

    if (outs == 0)
    {
        return 0;
    }

    if (outs > 20)
    {
        outs = 20;
    }

    // if the last raiser is allin on flop : we must count our odds for the turn AND the river
    // TODO : this is not correct, as we must also take into account the other players actions, and their stacks
    /*if (currentHand->getCurrentRoundState() == Flop)
    {
        const int lastRaiserId = currentHand->getLastRaiserId();

        if (lastRaiserId != -1)
        {
            std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserId);
            const std::vector<PlayerAction>& actions = lastRaiser->getCurrentHandActions().getActions(GameState::Flop);

            for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
                 itAction++)
                if ((*itAction) == ActionType::Allin)
                    return outsOddsTwoCard[outs];
        }
    }*/

    return outsOddsOneCard[outs];
}

const int getImplicitOdd(const PostFlopAnalysisFlags& state)
{

    // TODO compute implicit odd according to opponent's profiles, stack sizes and actions in this hand

    int implicitOdd = 0;

    return implicitOdd;
}

bool isDrawingProbOk(const PostFlopAnalysisFlags& postFlopState, const int potOdd)
{

    int implicitOdd = getImplicitOdd(postFlopState);
    int drawingProb = getDrawingProbability(postFlopState);

    if (drawingProb > 0)
    {

        if (drawingProb + implicitOdd >= potOdd)
        {

            return true;
        }
    }
    return false;
}

PlayerListIterator getPlayerListIteratorById(PlayerList list, unsigned id)
{
    return std::find_if(list->begin(), list->end(),
                        [id](const std::shared_ptr<Player>& p) { return p->getId() == id; });
}
std::shared_ptr<Player> getPlayerById(PlayerList list, unsigned id)
{
    for (auto i = list->begin(); i != list->end(); ++i)
    {
        if ((*i)->getId() == id)
            return *i;
    }
    return nullptr;
}
PlayerFsmListIterator getPlayerFsmListIteratorById(PlayerFsmList list, unsigned id)
{
    return std::find_if(list->begin(), list->end(),
                        [id](const std::shared_ptr<PlayerFsm>& p) { return p->getId() == id; });
}
std::shared_ptr<PlayerFsm> getPlayerFsmById(PlayerFsmList list, unsigned id)
{
    for (auto i = list->begin(); i != list->end(); ++i)
    {
        if ((*i)->getId() == id)
            return *i;
    }
    return nullptr;
}
void updateActingPlayersList(PlayerList& myActingPlayersList)
{
    GlobalServices::instance().logger().verbose("Updating myActingPlayersList...");

    PlayerListIterator it, it1;

    for (it = myActingPlayersList->begin(); it != myActingPlayersList->end();)
    {
        GlobalServices::instance().logger().verbose("Checking player: " + (*it)->getName() +
                                                    ", action: " + playerActionToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            GlobalServices::instance().logger().verbose(
                "Removing player: " + (*it)->getName() +
                " from myActingPlayersList due to action: " + playerActionToString((*it)->getLastAction().type));

            it = myActingPlayersList->erase(it);

            if (!myActingPlayersList->empty())
            {
                GlobalServices::instance().logger().verbose(
                    "myActingPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myActingPlayersList->begin())
                {
                    GlobalServices::instance().logger().verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myActingPlayersList->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger().verbose("myActingPlayersList is now empty after removal.");
            }
        }
        else
        {
            ++it;
        }
    }

    GlobalServices::instance().logger().verbose("Finished updating myActingPlayersList.");
}

void updateActingPlayersListFsm(PlayerFsmList& myActingPlayersListFsm)
{
    GlobalServices::instance().logger().verbose("Updating myActingPlayersListFsm...");

    PlayerFsmListIterator it, it1;

    for (it = myActingPlayersListFsm->begin(); it != myActingPlayersListFsm->end();)
    {
        GlobalServices::instance().logger().verbose("Checking player: " + (*it)->getName() +
                                                    ", action: " + playerActionToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            GlobalServices::instance().logger().verbose(
                "Removing player: " + (*it)->getName() +
                " from myActingPlayersListFsm due to action: " + playerActionToString((*it)->getLastAction().type));

            it = myActingPlayersListFsm->erase(it);

            if (!myActingPlayersListFsm->empty())
            {
                GlobalServices::instance().logger().verbose(
                    "myActingPlayersListFsm is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myActingPlayersListFsm->begin())
                {
                    GlobalServices::instance().logger().verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myActingPlayersListFsm->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger().verbose("myActingPlayersListFsm is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger().verbose(
                "Player: " + (*it)->getName() + " remains in myActingPlayersListFsm. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger().verbose("Finished updating myActingPlayersListFsm.");
}

std::string getPositionLabel(PlayerPosition p)
{

    switch (p)
    {

    case UnderTheGun:
        return "UnderTheGun";
        break;
    case UnderTheGunPlusOne:
        return "UnderTheGun+1";
        break;
    case UnderTheGunPlusTwo:
        return "UnderTheGun+2";
        break;
    case Middle:
        return "Middle";
        break;
    case MiddlePlusOne:
        return "Middle+1";
        break;
    case Late:
        return "Late";
        break;
    case Cutoff:
        return "Cutoff";
        break;
    case Button:
        return "Button";
        break;
    case ButtonSmallBlind:
        return "Button / Small Blind";
        break;
    case SmallBlind:
        return "Small Blind";
        break;
    case BigBlind:
        return "Big Blind";
        break;
    default:
        return "unknown";
        break;
    }
}

PlayerListIterator findPlayerOrThrow(PlayerList seats, unsigned id)
{
    auto it = getPlayerListIteratorById(seats, id);
    if (it == seats->end())
    {
        string ids;
        for (auto i = seats->begin(); i != seats->end(); ++i)
            ids += " " + to_string((*i)->getId());
        GlobalServices::instance().logger().error("Couldn't find player with id " + to_string(id) +
                                                  " in the seats list. List contains following ids :" + ids);
        throw Exception(__FILE__, __LINE__, EngineError::ActingPlayerNotFound);
    }
    return it;
}

PlayerListIterator nextActivePlayer(PlayerList seats, PlayerListIterator it)
{
    ++it;
    if (it == seats->end())
        it = seats->begin();
    while ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
    {
        ++it;
        if (it == seats->end())
            it = seats->begin();
    }
    return it;
}

PlayerFsmListIterator nextActivePlayerFsm(PlayerFsmList seats, PlayerFsmListIterator it)
{
    ++it;
    if (it == seats->end())
        it = seats->begin();
    while ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
    {
        ++it;
        if (it == seats->end())
            it = seats->begin();
    }
    return it;
}

std::shared_ptr<player::PlayerFsm> getFirstPlayerToActPostFlop(const HandFsm& hand)
{
    auto actingPlayers = hand.getActingPlayersList();

    if (actingPlayers->empty())
        return nullptr;

    for (auto it = actingPlayers->begin(); it != actingPlayers->end(); ++it)
    {
        if ((*it)->getPosition() == PlayerPosition::Button || (*it)->getPosition() == PlayerPosition::ButtonSmallBlind)
        {
            auto nextIt = std::next(it);
            if (nextIt == actingPlayers->end())
                nextIt = actingPlayers->begin();

            return *nextIt;
        }
    }

    return actingPlayers->front();
}

bool hasPosition(PlayerPosition position, PlayerFsmList actingPlayers)
{
    // return true if position is last to play, false if not

    bool hasPosition = true;

    PlayerFsmListConstIterator itC;

    for (itC = actingPlayers->begin(); itC != actingPlayers->end(); ++itC)
    {

        if ((*itC)->getPosition() > position)
        {
            hasPosition = false;
        }
    }

    return hasPosition;
}

std::vector<ActionType> getValidActionsForPlayer(const HandFsm& hand, int playerId)
{
    int smallBlind = hand.getSmallBlind();

    return getValidActionsForPlayer(hand.getActingPlayersList(), playerId, *hand.getBettingActions(), smallBlind,
                                    hand.getGameState());
}

std::vector<ActionType> getValidActionsForPlayer(const PlayerFsmList& actingPlayersList, int playerId,
                                                 const BettingActions& bettingActions, int smallBlind,
                                                 const GameState gameState)
{
    std::vector<ActionType> validActions;

    // Find the player in the acting players list
    auto player = getPlayerFsmById(actingPlayersList, playerId);
    if (!player)
    {
        return validActions; // Return empty list if player not found
    }

    const int currentHighestBet = bettingActions.getRoundHighestSet();
    const int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(gameState);
    const int playerCash = player->getCash();

    // Fold is almost always available (except in some edge cases like being all-in)
    if (playerCash > 0 || playerBet < currentHighestBet)
    {
        validActions.push_back(ActionType::Fold);
    }

    // Check if player can check (when no bet to call)
    if (playerBet == currentHighestBet)
    {
        validActions.push_back(ActionType::Check);
    }

    // Check if player can call (when there's a bet to call and player has chips)
    if (playerBet < currentHighestBet && playerCash > 0)
    {
        validActions.push_back(ActionType::Call);
    }

    // Check if player can bet (when no current bet and player has chips)
    if (currentHighestBet == 0 && playerCash > 0)
    {
        validActions.push_back(ActionType::Bet);
    }

    // Check if player can raise (when there's a current bet and player has enough chips)
    if (currentHighestBet > 0 && playerCash > 0)
    {
        // Get minimum raise amount based on game rules
        int minRaise = bettingActions.getMinRaise(smallBlind);
        int minRaiseAmount = currentHighestBet + minRaise;
        int extraChipsRequired = minRaiseAmount - playerBet;

        if (extraChipsRequired <= playerCash)
        {
            validActions.push_back(ActionType::Raise);
        }
    }

    // All-in is available if player has chips
    if (playerCash > 0)
    {
        validActions.push_back(ActionType::Allin);
    }

    return validActions;
}

bool validatePlayerAction(const PlayerFsmList& actingPlayersList, const PlayerAction& action,
                          const BettingActions& bettingActions, int smallBlind, const GameState gameState)
{
    auto player = getPlayerFsmById(actingPlayersList, action.playerId);
    if (!player)
    {
        GlobalServices::instance().logger().error(gameStateToString(gameState) + ": player with id " +
                                                  std::to_string(action.playerId) + " not found in actingPlayersList");
        return false;
    }

    // Get valid actions for the player
    std::vector<ActionType> validActions =
        getValidActionsForPlayer(actingPlayersList, action.playerId, bettingActions, smallBlind, gameState);

    // Check if the action type is in the list of valid actions
    bool isActionTypeValid = std::find(validActions.begin(), validActions.end(), action.type) != validActions.end();

    if (!isActionTypeValid)
    {
        GlobalServices::instance().logger().error(gameStateToString(gameState) + ": Invalid action type for player " +
                                                  player->getName() + " : " + playerActionToString(action.type));
        return false;
    }

    // Additional validation for actions that require specific amounts
    const int currentHighestBet = bettingActions.getRoundHighestSet();
    const int playerBet = player->getCurrentHandActions().getRoundTotalBetAmount(gameState);

    bool isActionValid = true;

    switch (action.type)
    {
    case ActionType::Fold:
        // Fold doesn't require amount validation
        break;

    case ActionType::Check:
        isActionValid = (action.amount == 0);
        break;

    case ActionType::Call:
        // Amount will be calculated by the system
        break;

    case ActionType::Bet:
        isActionValid = (action.amount > 0 && action.amount <= player->getCash());
        break;

    case ActionType::Raise:
    {
        isActionValid = (action.amount > currentHighestBet);

        int minRaise = bettingActions.getMinRaise(smallBlind);
        isActionValid = (isActionValid && action.amount >= currentHighestBet + minRaise);

        const int extraChipsRequired = action.amount - playerBet;
        isActionValid = (isActionValid && extraChipsRequired <= player->getCash());
        break;
    }
    case ActionType::Allin:
        // All-in doesn't require specific amount validation
        break;

    default:
        isActionValid = false;
    }

    if (!isActionValid)
    {
        GlobalServices::instance().logger().error(
            gameStateToString(gameState) + ": Invalid action amount for player " + std::to_string(action.playerId) +
            " : " + playerActionToString(action.type) + " with amount = " + std::to_string(action.amount));
    }

    return isActionValid;
}

// Helper to compute the relative offset in the circular table
int circularOffset(int fromId, int toId, const PlayerFsmList& players)
{
    int distance = 0;
    bool found = false;
    for (auto& p : *players)
    {
        if (p->getId() == fromId)
            found = true;
        else if (found)
            distance++;
        if (p->getId() == toId)
            break;
    }
    return distance;
}

PlayerPosition computePositionFromOffset(int offset, int nbPlayers)
{
    // Dealer always at 0
    switch (nbPlayers)
    {
    case 2:
        return offset == 0 ? ButtonSmallBlind : BigBlind;
    case 3:
        return (offset == 0) ? Button : (offset == 1 ? SmallBlind : BigBlind);
    case 4:
        return (offset == 0) ? Button : (offset == 1 ? SmallBlind : (offset == 2 ? BigBlind : UnderTheGun));
    case 5:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        return Cutoff;
    case 6:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return Middle;
        return Cutoff;
    case 7:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return Middle;
        if (offset == 5)
            return Cutoff;
        return Late;
    case 8:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return UnderTheGunPlusOne;
        if (offset == 5)
            return Middle;
        if (offset == 6)
            return Cutoff;
        return Late;
    case 9:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return UnderTheGunPlusOne;
        if (offset == 5)
            return Middle;
        if (offset == 6)
            return MiddlePlusOne;
        if (offset == 7)
            return Cutoff;
        return Late;
    case 10:
        if (offset == 0)
            return Button;
        if (offset == 1)
            return SmallBlind;
        if (offset == 2)
            return BigBlind;
        if (offset == 3)
            return UnderTheGun;
        if (offset == 4)
            return UnderTheGunPlusOne;
        if (offset == 5)
            return UnderTheGunPlusTwo;
        if (offset == 6)
            return Middle;
        if (offset == 7)
            return MiddlePlusOne;
        if (offset == 8)
            return Cutoff;
        return Late;
    default:
        return Unknown;
    }
}

bool isDealerPosition(PlayerPosition p)
{
    return p == PlayerPosition::Button || p == PlayerPosition::ButtonSmallBlind;
}

bool isSmallBlindPosition(PlayerPosition p)
{
    return p == PlayerPosition::SmallBlind || p == PlayerPosition::ButtonSmallBlind;
}

bool isRoundComplete(HandFsm& hand)
{
    assert(hand.getGameState() != GameState::None);

    for (auto player = hand.getActingPlayersList()->begin(); player != hand.getActingPlayersList()->end(); ++player)
    {
        GlobalServices::instance().logger().verbose("checking if round " + gameStateToString(hand.getGameState()) +
                                                    " is complete : Checking player: " + (*player)->getName());

        if ((*player)->getLastAction().type == ActionType::None ||
            (*player)->getLastAction().type == ActionType::PostBigBlind ||
            (*player)->getLastAction().type == ActionType::PostSmallBlind)
        {
            GlobalServices::instance().logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() +
                                                        " did not act.");
            return false;
        }

        GlobalServices::instance().logger().verbose(
            "  player round bet amount: " +
            std::to_string((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState())) +
            ", hand total bet amount : " + std::to_string((*player)->getCurrentHandActions().getHandTotalBetAmount()) +
            " vs current round highest bet: " + std::to_string(hand.getBettingActions()->getRoundHighestSet()));

        if ((*player)->getCurrentHandActions().getRoundTotalBetAmount(hand.getGameState()) <
            hand.getBettingActions()->getRoundHighestSet())
        {
            GlobalServices::instance().logger().verbose("  ROUND NOT COMPLETE, as player " + (*player)->getName() +
                                                        " has not matched the highest bet yet.");
            return false;
        }
    }
    GlobalServices::instance().logger().verbose("  ROUND " + gameStateToString(hand.getGameState()) + " COMPLETE");
    return true;
}

std::unique_ptr<pkt::core::IHandState> computeBettingRoundNextState(pkt::core::HandFsm& hand,
                                                                    const pkt::core::GameEvents& events,
                                                                    pkt::core::GameState currentState)
{
    // If less than 2 players are still in hand (haven't folded), go directly to showdown
    if (hand.getPlayersInHandList()->size() < 2)
    {
        return std::make_unique<pkt::core::PostRiverState>(events);
    }

    // If all remaining players are all-in (no one can act further), go directly to showdown
    if (hand.getActingPlayersList()->empty() && hand.getPlayersInHandList()->size() >= 1)
    {
        return std::make_unique<pkt::core::PostRiverState>(events);
    }

    // If round is complete, check if we can continue betting
    if (isRoundComplete(hand))
    {
        // If only one or no players can still act, go directly to showdown
        if (hand.getActingPlayersList()->size() <= 1)
        {
            return std::make_unique<pkt::core::PostRiverState>(events);
        }

        // Multiple players can still act, determine next state based on current state
        switch (currentState)
        {
        case pkt::core::GameState::Preflop:
            return std::make_unique<pkt::core::FlopState>(events);
        case pkt::core::GameState::Flop:
            return std::make_unique<pkt::core::TurnState>(events);
        case pkt::core::GameState::Turn:
            return std::make_unique<pkt::core::RiverState>(events);
        case pkt::core::GameState::River:
            return std::make_unique<pkt::core::PostRiverState>(events);
        default:
            // Shouldn't happen for betting rounds
            return std::make_unique<pkt::core::PostRiverState>(events);
        }
    }

    return nullptr; // Stay in current state - more betting needed
}

} // namespace pkt::core::player
