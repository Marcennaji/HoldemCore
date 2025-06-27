#include "CardUtilities.h"
#include <sstream>
#include <stdexcept>

namespace pkt::core
{

const std::string CardUtilities::CARD_STRING_VALUES[] = {
    "2d", "3d", "4d", "5d", "6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd", "Ad", "2h", "3h", "4h", "5h", "6h",
    "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh", "Ah", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s", "Ts", "Js",
    "Qs", "Ks", "As", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc", "Ac"};

const std::map<std::string, int> CardUtilities::CARD_STRING_TO_INDEX = {
    {"2d", 0},  {"3d", 1},  {"4d", 2},  {"5d", 3},  {"6d", 4},  {"7d", 5},  {"8d", 6},  {"9d", 7},  {"Td", 8},
    {"Jd", 9},  {"Qd", 10}, {"Kd", 11}, {"Ad", 12}, {"2h", 13}, {"3h", 14}, {"4h", 15}, {"5h", 16}, {"6h", 17},
    {"7h", 18}, {"8h", 19}, {"9h", 20}, {"Th", 21}, {"Jh", 22}, {"Qh", 23}, {"Kh", 24}, {"Ah", 25}, {"2s", 26},
    {"3s", 27}, {"4s", 28}, {"5s", 29}, {"6s", 30}, {"7s", 31}, {"8s", 32}, {"9s", 33}, {"Ts", 34}, {"Js", 35},
    {"Qs", 36}, {"Ks", 37}, {"As", 38}, {"2c", 39}, {"3c", 40}, {"4c", 41}, {"5c", 42}, {"6c", 43}, {"7c", 44},
    {"8c", 45}, {"9c", 46}, {"Tc", 47}, {"Jc", 48}, {"Qc", 49}, {"Kc", 50}, {"Ac", 51}};

const std::map<std::string, int> CardUtilities::CARD_STRING_ORDERING = {
    {"2d", 2},  {"3d", 3},  {"4d", 4},  {"5d", 5},  {"6d", 6},  {"7d", 7},  {"8d", 8},  {"9d", 9},  {"Td", 10},
    {"Jd", 11}, {"Qd", 12}, {"Kd", 13}, {"Ad", 14}, {"2c", 2},  {"3c", 3},  {"4c", 4},  {"5c", 5},  {"6c", 6},
    {"7c", 7},  {"8c", 8},  {"9c", 9},  {"Tc", 10}, {"Jc", 11}, {"Qc", 12}, {"Kc", 13}, {"Ac", 14}, {"2s", 2},
    {"3s", 3},  {"4s", 4},  {"5s", 5},  {"6s", 6},  {"7s", 7},  {"8s", 8},  {"9s", 9},  {"Ts", 10}, {"Js", 11},
    {"Qs", 12}, {"Ks", 13}, {"As", 14}, {"2h", 2},  {"3h", 3},  {"4h", 4},  {"5h", 5},  {"6h", 6},  {"7h", 7},
    {"8h", 8},  {"9h", 9},  {"Th", 10}, {"Jh", 11}, {"Qh", 12}, {"Kh", 13}, {"Ah", 14}};

int CardUtilities::getCardValue(const std::string& cardString)
{
    auto it = CARD_STRING_ORDERING.find(cardString);
    if (it == CARD_STRING_ORDERING.end())
    {
        throw std::invalid_argument("Invalid card string: " + cardString);
    }
    return it->second;
}
std::string CardUtilities::getCardStringValue(const std::vector<int>& cardValues)
{
    return getCardStringValue(cardValues.data(), static_cast<int>(cardValues.size()));
}

std::string CardUtilities::getCardStringValue(const int* cardValues, int size)
{
    std::ostringstream result;
    for (int i = 0; i < size; ++i)
    {
        if (i > 0)
            result << " ";
        result << getCardString(cardValues[i]);
    }
    return result.str();
}

const std::string& CardUtilities::getCardString(int cardIndex)
{
    if (cardIndex < 0 || cardIndex >= 52)
    {
        throw std::out_of_range("Card index out of range: " + std::to_string(cardIndex));
    }
    return CARD_STRING_VALUES[cardIndex];
}

int CardUtilities::getCardIndex(const std::string& cardString)
{
    auto it = CARD_STRING_TO_INDEX.find(cardString);
    if (it == CARD_STRING_TO_INDEX.end())
    {
        throw std::invalid_argument("Invalid card string: " + cardString);
    }
    return it->second;
}

bool CardUtilities::isValidCardString(const std::string& cardString)
{
    return CARD_STRING_TO_INDEX.find(cardString) != CARD_STRING_TO_INDEX.end();
}

} // namespace pkt::core