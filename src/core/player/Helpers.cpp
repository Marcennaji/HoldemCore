
#include "Helpers.h"
#include "core/engine/CardUtilities.h"
#include "core/engine/Exception.h"
#include "core/engine/model/ButtonState.h"
#include "core/player/PlayerFsm.h"
#include "core/player/deprecated/Player.h"
#include "core/services/GlobalServices.h"

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
            const std::vector<PlayerAction>& actions = lastRaiser->getCurrentHandActions().getFlopActions();

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
void updateRunningPlayersList(PlayerList& myRunningPlayersList)
{
    GlobalServices::instance().logger()->verbose("Updating myRunningPlayersList...");

    PlayerListIterator it, it1;

    for (it = myRunningPlayersList->begin(); it != myRunningPlayersList->end();)
    {
        GlobalServices::instance().logger()->verbose("Checking player: " + (*it)->getName() +
                                                     ", action: " + playerActionToString((*it)->getAction()));

        if ((*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::Allin)
        {
            GlobalServices::instance().logger()->verbose(
                "Removing player: " + (*it)->getName() +
                " from myRunningPlayersList due to action: " + playerActionToString((*it)->getAction()));

            it = myRunningPlayersList->erase(it);

            if (!myRunningPlayersList->empty())
            {
                GlobalServices::instance().logger()->verbose(
                    "myRunningPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myRunningPlayersList->begin())
                {
                    GlobalServices::instance().logger()->verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myRunningPlayersList->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger()->verbose("myRunningPlayersList is now empty after removal.");
            }
        }
        else
        {
            ++it;
        }
    }

    GlobalServices::instance().logger()->verbose("Finished updating myRunningPlayersList.");
}

void updateRunningPlayersListFsm(PlayerFsmList& myRunningPlayersListFsm)
{
    GlobalServices::instance().logger()->verbose("Updating myRunningPlayersListFsm...");

    PlayerFsmListIterator it, it1;

    for (it = myRunningPlayersListFsm->begin(); it != myRunningPlayersListFsm->end();)
    {
        GlobalServices::instance().logger()->verbose("Checking player: " + (*it)->getName() +
                                                     ", action: " + playerActionToString((*it)->getAction()));

        if ((*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::Allin)
        {
            GlobalServices::instance().logger()->verbose(
                "Removing player: " + (*it)->getName() +
                " from myRunningPlayersListFsm due to action: " + playerActionToString((*it)->getAction()));

            it = myRunningPlayersListFsm->erase(it);

            if (!myRunningPlayersListFsm->empty())
            {
                GlobalServices::instance().logger()->verbose(
                    "myRunningPlayersListFsm is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myRunningPlayersListFsm->begin())
                {
                    GlobalServices::instance().logger()->verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myRunningPlayersListFsm->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger()->verbose("myRunningPlayersListFsm is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger()->verbose(
                "Player: " + (*it)->getName() + " remains in myRunningPlayersListFsm. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger()->verbose("Finished updating myRunningPlayersListFsm.");
}

std::string getPositionLabel(PlayerPosition p)
{

    switch (p)
    {

    case UTG:
        return "UTG";
        break;
    case UtgPlusOne:
        return "UtgPlusOne";
        break;
    case UtgPlusTwo:
        return "UtgPlusTwo";
        break;
    case MIDDLE:
        return "MIDDLE";
        break;
    case MiddlePlusOne:
        return "MiddlePlusOne";
        break;
    case LATE:
        return "LATE";
        break;
    case CUTOFF:
        return "CUTOFF";
        break;
    case BUTTON:
        return "BUTTON";
        break;
    case SB:
        return "SB";
        break;
    case BB:
        return "BB";
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
        GlobalServices::instance().logger()->error("Couldn't find player with id " + to_string(id) +
                                                   " in the seats list. List contains following ids :" + ids);
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }
    return it;
}

PlayerListIterator nextActivePlayer(PlayerList seats, PlayerListIterator it)
{
    ++it;
    if (it == seats->end())
        it = seats->begin();
    while ((*it)->getAction() == ActionType::Fold || (*it)->getAction() == ActionType::Allin)
    {
        ++it;
        if (it == seats->end())
            it = seats->begin();
    }
    return it;
}
bool hasPosition(PlayerPosition position, PlayerFsmList runningPlayers)
{
    // return true if position is last to play, false if not

    bool hasPosition = true;

    PlayerFsmListConstIterator itC;

    for (itC = runningPlayers->begin(); itC != runningPlayers->end(); ++itC)
    {

        if ((*itC)->getPosition() > position)
        {
            hasPosition = false;
        }
    }

    return hasPosition;
}
} // namespace pkt::core::player
