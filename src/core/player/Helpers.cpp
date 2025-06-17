
#include "Helpers.h"
#include <third_party/psim/psim.hpp>
#include "core/player/Player.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

namespace pkt::core::player
{
using namespace std;

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId)
{
    std::vector<std::shared_ptr<Player>> v(players.begin(), players.end());
    auto it = std::find_if(v.begin(), v.end(), [=](auto& p) { return p->getID() == humanId; });
    if (it != v.end())
        std::swap(v.front(), *it);

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

        if (CardsValue::CardStringOrdering[boardCard] > CardsValue::CardStringOrdering[card])
            n++;
    }
    return n;
}

bool isCardsInRange(string card1, string card2, string ranges)
{

    // process individual ranges, from a string looking like "33,66+,T8o,ATs+,QJs,JTs,AQo+, KcJh"

    // card1 = "7h";
    // card2 = "6h";
    // ranges = "74s+";
    // ranges = CUTOFF_STARTING_RANGE[10];

    // first card must be the highest
    if (CardsValue::CardStringOrdering[card1] < CardsValue::CardStringOrdering[card2])
    {
        string tmp = card1;
        card1 = card2;
        card2 = tmp;
    }

    const char* c1 = card1.c_str();
    const char* c2 = card2.c_str();

    std::istringstream oss(ranges);
    std::string token;

    while (getline(oss, token, ','))
    {

        if (token.size() == 0)
            continue;

        if (token.size() == 1 || token.size() > 4)
        {
            return false;
        }
        const char* range = token.c_str();

        if (token.size() == 2)
        { // an exact pair, like 55 or AA

            if (c1[0] == range[0] && c2[0] == range[1])
            {
                return true;
            }
        }
        if (token.size() == 3)
        {

            if (range[0] != range[1] && range[2] == 's')
            { // range is an exact suited hand, like QJs or 56s
                if (((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) &&
                    (c1[1] == c2[1]))
                {
                    return true;
                }
            }

            if (range[0] != range[1] && range[2] == 'o')
            { // range is an exact offsuited cards, like KTo or 24o
                if (((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) &&
                    (c1[1] != c2[1]))
                {
                    return true;
                }
            }
            if (range[0] == range[1] && range[2] == '+')
            { // range is a pair and above, like 99+
                if (CardsValue::CardStringOrdering[card1] == CardsValue::CardStringOrdering[card2])
                {
                    // we have a pair. Is it above the minimum value ?
                    if (CardsValue::CardStringOrdering[card1] >= CardsValue::CardStringOrdering[getFakeCard(range[0])])
                        return true;
                }
            }
        }
        if (token.size() == 4)
        {

            if (range[0] != range[1] && range[2] == 'o' && range[3] == '+')
            {
                // range is offsuited and above, like AQo+
                if (CardsValue::CardStringOrdering[card1] == CardsValue::CardStringOrdering[getFakeCard(range[0])] &&
                    CardsValue::CardStringOrdering[card2] >= CardsValue::CardStringOrdering[getFakeCard(range[1])] &&
                    CardsValue::CardStringOrdering[card2] < CardsValue::CardStringOrdering[card1] && c1[1] != c2[1])
                    return true;
            }
            if (range[0] != range[1] && range[2] == 's' && range[3] == '+')
            {
                // range is suited and above, like AJs+
                if (CardsValue::CardStringOrdering[card1] == CardsValue::CardStringOrdering[getFakeCard(range[0])] &&
                    CardsValue::CardStringOrdering[card2] >= CardsValue::CardStringOrdering[getFakeCard(range[1])] &&
                    CardsValue::CardStringOrdering[card2] < CardsValue::CardStringOrdering[card1] && c1[1] == c2[1])
                    return true;
            }
            if (range[2] != 's' && range[2] != 'o')
            {
                // range is an exact hand, like AhKc
                string exactCard1;
                exactCard1 += range[0];
                exactCard1 += range[1];

                string exactCard2;
                exactCard2 += range[2];
                exactCard2 += range[3];

                if ((card1 == exactCard1 && card2 == exactCard2) || (card1 == exactCard2 && card2 == exactCard1))
                    return true;
            }
        }
    }

    return false;
}

// returns a % chance, for a winning draw
const int getDrawingProbability(const PostFlopState& postFlopState)
{

    if (!postFlopState.UsesFirst && !postFlopState.UsesSecond)
        return 0;

    int outs = 0;

    // do not count outs for straight or flush, is the board is paired

    if (!postFlopState.IsFullHousePossible)
        outs = postFlopState.StraightOuts + postFlopState.FlushOuts + postFlopState.BetterOuts;
    else
        outs = postFlopState.BetterOuts;

    if (outs == 0)
        return 0;

    if (outs > 20)
        outs = 20;

    // if the last raiser is allin on flop : we must count our odds for the turn AND the river
    // TODO : this is not correct, as we must also take into account the other players actions, and their stacks
    /*if (currentHand->getCurrentRound() == GAME_STATE_FLOP)
    {
        const int lastRaiserID = currentHand->getLastRaiserID();

        if (lastRaiserID != -1)
        {
            std::shared_ptr<Player> lastRaiser = getPlayerByUniqueId(lastRaiserID);
            const std::vector<PlayerAction>& actions = lastRaiser->getCurrentHandActions().getFlopActions();

            for (std::vector<PlayerAction>::const_iterator itAction = actions.begin(); itAction != actions.end();
                 itAction++)
                if ((*itAction) == PLAYER_ACTION_ALLIN)
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

} // namespace pkt::core::player
