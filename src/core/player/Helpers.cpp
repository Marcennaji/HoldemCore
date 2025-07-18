
#include "Helpers.h"
#include <third_party/psim/psim.hpp>
#include "core/engine/CardUtilities.h"
#include "core/player/Player.h"

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
const int getDrawingProbability(const PostFlopState& postFlopState)
{

    if (!postFlopState.UsesFirst && !postFlopState.UsesSecond)
    {
        return 0;
    }

    int outs = 0;

    // do not count outs for straight or flush, is the board is paired

    if (!postFlopState.IsFullHousePossible)
    {
        outs = postFlopState.StraightOuts + postFlopState.FlushOuts + postFlopState.BetterOuts;
    }
    else
    {
        outs = postFlopState.BetterOuts;
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
    /*if (currentHand->getCurrentRoundState() == GameStateFlop)
    {
        const int lastRaiserId = currentHand->getLastRaiserId();

        if (lastRaiserId != -1)
        {
            std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserId);
            const std::vector<PlayerAction>& actions = lastRaiser->getCurrentHandActions().getFlopActions();

            for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
                 itAction++)
                if ((*itAction) == PlayerActionAllin)
                    return outsOddsTwoCard[outs];
        }
    }*/

    return outsOddsOneCard[outs];
}

const int getImplicitOdd(const PostFlopState& state)
{

    // TODO compute implicit odd according to opponent's profiles, stack sizes and actions in this hand

    int implicitOdd = 0;

    return implicitOdd;
}

bool isDrawingProbOk(const PostFlopState& postFlopState, const int potOdd)
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

} // namespace pkt::core::player
