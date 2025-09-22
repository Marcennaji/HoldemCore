// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "CardRangeAnalyzer.h"
#include "core/engine/cards/CardUtilities.h"

#include <algorithm>
#include <cstring>
#include <sstream>

using namespace std;

namespace pkt::core::cards
{

bool CardRangeAnalyzer::isCardsInRange(const std::string& card1, const std::string& card2, const std::string& ranges)
{
    string c1 = card1;
    string c2 = card2;

    // Ensure the first card is the highest
    ensureHighestCard(c1, c2);

    const char* card1Cstr = c1.c_str();
    const char* card2Cstr = c2.c_str();

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

        if (isExactPair(card1Cstr, card2Cstr, range))
        {
            return true;
        }

        if (isExactSuitedHand(card1Cstr, card2Cstr, range))
        {
            return true;
        }

        if (isExactOffsuitedHand(card1Cstr, card2Cstr, range))
        {
            return true;
        }

        if (isPairAndAbove(c1, c2, range))
        {
            return true;
        }

        if (isOffsuitedAndAbove(c1, c2, card1Cstr, card2Cstr, range))
        {
            return true;
        }

        if (isSuitedAndAbove(c1, c2, card1Cstr, card2Cstr, range))
        {
            return true;
        }

        if (isExactHand(c1, c2, range))
        {
            return true;
        }
    }

    return false;
}

int CardRangeAnalyzer::getBoardCardsHigherThan(const std::string& stringBoard, const std::string& card)
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

// Private helper functions
std::string CardRangeAnalyzer::getFakeCard(char c)
{
    char tmp[3];
    tmp[0] = c;
    tmp[1] = 'c';
    tmp[2] = '\0';
    return string(tmp);
}

void CardRangeAnalyzer::ensureHighestCard(std::string& card1, std::string& card2)
{
    if (CardUtilities::getCardValue(card1) < CardUtilities::getCardValue(card2))
    {
        std::swap(card1, card2);
    }
}

bool CardRangeAnalyzer::isValidRange(const std::string& token)
{
    return !(token.size() == 1 || token.size() > 4);
}

bool CardRangeAnalyzer::isExactPair(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 2 && c1[0] == range[0] && c2[0] == range[1]);
}

bool CardRangeAnalyzer::isExactSuitedHand(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 3 && range[2] == 's' &&
            ((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) && (c1[1] == c2[1]));
}

bool CardRangeAnalyzer::isExactOffsuitedHand(const char* c1, const char* c2, const char* range)
{
    return (strlen(range) == 3 && range[2] == 'o' &&
            ((c1[0] == range[0] && c2[0] == range[1]) || (c1[0] == range[1] && c2[0] == range[0])) && (c1[1] != c2[1]));
}

bool CardRangeAnalyzer::isPairAndAbove(const std::string& card1, const std::string& card2, const char* range)
{
    return (strlen(range) == 3 && range[0] == range[1] && range[2] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(card2) &&
            CardUtilities::getCardValue(card1) >= CardUtilities::getCardValue(getFakeCard(range[0])));
}

bool CardRangeAnalyzer::isOffsuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1,
                                            const char* c2, const char* range)
{
    return (strlen(range) == 4 && range[2] == 'o' && range[3] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(getFakeCard(range[0])) &&
            CardUtilities::getCardValue(card2) >= CardUtilities::getCardValue(getFakeCard(range[1])) &&
            CardUtilities::getCardValue(card2) < CardUtilities::getCardValue(card1) && c1[1] != c2[1]);
}

bool CardRangeAnalyzer::isSuitedAndAbove(const std::string& card1, const std::string& card2, const char* c1,
                                         const char* c2, const char* range)
{
    return (strlen(range) == 4 && range[2] == 's' && range[3] == '+' &&
            CardUtilities::getCardValue(card1) == CardUtilities::getCardValue(getFakeCard(range[0])) &&
            CardUtilities::getCardValue(card2) >= CardUtilities::getCardValue(getFakeCard(range[1])) &&
            CardUtilities::getCardValue(card2) < CardUtilities::getCardValue(card1) && c1[1] == c2[1]);
}

bool CardRangeAnalyzer::isExactHand(const std::string& card1, const std::string& card2, const char* range)
{
    if (strlen(range) == 4 && range[2] != 's' && range[2] != 'o')
    {
        string exactCard1 = {range[0], range[1]};
        string exactCard2 = {range[2], range[3]};

        return (card1 == exactCard1 && card2 == exactCard2) || (card1 == exactCard2 && card2 == exactCard1);
    }
    return false;
}

} // namespace pkt::core::cards
