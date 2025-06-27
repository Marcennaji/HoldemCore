// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "CardsValue.h"
#include <third_party/psim/psim.hpp>
#include "core/services/GlobalServices.h"

// translate card int code, to string equivalent

namespace pkt::core
{

const std::string CardsValue::CardStringValue[] = {
    "2d", "3d", "4d", "5d", "6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd", "Ad", "2h", "3h", "4h", "5h", "6h",
    "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh", "Ah", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s", "Ts", "Js",
    "Qs", "Ks", "As", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc", "Ac"};

std::map<std::string, int> CardsValue::CardStringOrdering = {
    {"2d", 2},  {"3d", 3},  {"4d", 4},  {"5d", 5},  {"6d", 6},  {"7d", 7},  {"8d", 8},  {"9d", 9},  {"Td", 10},
    {"Jd", 11}, {"Qd", 12}, {"Kd", 13}, {"Ad", 14}, {"2c", 2},  {"3c", 3},  {"4c", 4},  {"5c", 5},  {"6c", 6},
    {"7c", 7},  {"8c", 8},  {"9c", 9},  {"Tc", 10}, {"Jc", 11}, {"Qc", 12}, {"Kc", 13}, {"Ac", 14}, {"2s", 2},
    {"3s", 3},  {"4s", 4},  {"5s", 5},  {"6s", 6},  {"7s", 7},  {"8s", 8},  {"9s", 9},  {"Ts", 10}, {"Js", 11},
    {"Qs", 12}, {"Ks", 13}, {"As", 14}, {"2h", 2},  {"3h", 3},  {"4h", 4},  {"5h", 5},  {"6h", 6},  {"7h", 7},
    {"8h", 8},  {"9h", 9},  {"Th", 10}, {"Jh", 11}, {"Qh", 12}, {"Kh", 13}, {"Ah", 14}};

unsigned int CardsValue::evaluateHand(const char* hand)
{
    GlobalServices::instance().logger()->info("Evaluating hand : " + std::string(hand));

    unsigned int result = rankHand(hand);

    GlobalServices::instance().logger()->info("Hand evaluation ranking result: " + std::to_string(result));
    return result;
}

std::string CardsValue::getCardStringValue(std::vector<int> cardValues)
{
    std::string result;
    for (const auto& cardValue : cardValues)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += CardStringValue[cardValue];
    }
    return result;
}
std::string CardsValue::getCardStringValue(const int* cardValues, int size)
{
    std::string result;
    for (int i = 0; i < size; ++i)
    {
        if (!result.empty())
        {
            result += " ";
        }
        result += CardStringValue[cardValues[i]];
    }
    return result;
}

} // namespace pkt::core
