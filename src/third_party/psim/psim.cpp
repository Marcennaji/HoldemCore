// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "psim.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "simulate.hpp"

namespace
{
/// Pass in hand array of ints and this'll process it into a useful structure.
Simulate::Hand IntsToHand(const int* hand)
{
    size_t cards = 2;

    CardMask common;
    CardMask_RESET(common);

    for (size_t card = 2; card < 7; ++card)      // Loop through the common cards.
        if (hand[card] >= 0 && hand[card] <= 51) // If it's a real card, set it.
        {
            CardMask_SET(common, hand[card]);
            ++cards;
        }

    // Grab our hole cards.
    CardMask fh, sh;
    CardMask_RESET(fh);
    CardMask_RESET(sh);
    CardMask_SET(fh, hand[0]);
    CardMask_SET(sh, hand[1]);

    return Simulate::Hand(fh, sh, common, cards);
}

/// Pass in a string like "Ah Ad 2s Tc 4d 5h" and this'll process it into a useful structure.
Simulate::Hand stringToHand(const char* hand)
{
    std::vector<CardMask> cardVec;
    cardVec.reserve(7);

    char cs[] = "00";

    size_t len = std::strlen(hand);

    for (size_t x = 0; x < len; ++x)
    {
        char c = hand[x];

        switch (c)
        {
        case 'A':
        case 'a':
        case 'K':
        case 'k':
        case 'Q':
        case 'q':
        case 'J':
        case 'j':
        case 'T':
        case 't':
        case '9':
        case '8':
        case '7':
        case '6':
        case '5':
        case '4':
        case '3':
        case '2':
            cs[0] = c;
            break;

        case 'S':
        case 's':
        case 'H':
        case 'h':
        case 'D':
        case 'd':
        case 'C':
        case 'c':
            cs[1] = c;
            cardVec.push_back(Simulate::MakeCard(cs));
            break;
        }
    }

    CardMask common;
    CardMask_RESET(common);

    for (size_t x = 2; x < cardVec.size(); x++)
    {
        CardMask_OR(common, common, cardVec[x]);
    }

    return Simulate::Hand(cardVec[0], cardVec[1], common, cardVec.size());
}
} // namespace

void getHandState(const char* hand, PostFlopState* state)
{
    Simulate::Hand shand = stringToHand(hand);
    Simulate::getHandState(shand, state);
}

void getHandState(const int* hand, PostFlopState* state)
{
    Simulate::Hand shand = IntsToHand(hand);
    Simulate::getHandState(shand, state);
}

unsigned int rankHand(const char* hand)
{
    Simulate::Hand shand = stringToHand(hand);
    return Simulate::rankHand(shand);
}

unsigned int rankHand(const int* hand)
{
    Simulate::Hand shand = IntsToHand(hand);
    return Simulate::rankHand(shand);
}

void simulateHand(const char* hand, SimResults* results, float lowRange, float highRange, unsigned int boards)
{
    Simulate::Hand shand = stringToHand(hand);
    Simulate::simulateHand(shand, results, lowRange, highRange, boards);
}

void simulateHand(const int* hand, SimResults* results, float lowRange, float highRange, unsigned int boards)
{
    Simulate::Hand shand = IntsToHand(hand);
    Simulate::simulateHand(shand, results, lowRange, highRange, boards);
}

void simulateHandMulti(const char* hand, SimResults* results, unsigned int boards, unsigned int hands,
                       unsigned int numOpponents)
{
    Simulate::Hand shand = stringToHand(hand);
    Simulate::simulateHandMulti(shand, results, boards, hands, numOpponents);
}

void simulateHandMulti(const int* hand, SimResults* results, unsigned int boards, unsigned int hands,
                       unsigned int numOpponents)
{
    Simulate::Hand shand = IntsToHand(hand);
    Simulate::simulateHandMulti(shand, results, boards, hands, numOpponents);
}
