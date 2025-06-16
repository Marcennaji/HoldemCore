#include <core/player/range/RangeParser.h>
#include <iostream>
#include <sstream>

namespace pkt::core::player
{
using namespace std;

std::vector<std::string> RangeParser::getRangeAtomicValues(const std::string& ranges, const bool returnRange)
{
    std::vector<std::string> result;

    std::istringstream oss(ranges);
    std::string token;

    while (std::getline(oss, token, ','))
    {
        if (token.empty())
            continue;

        if (token.size() == 1 || token.size() > 4)
        {
            std::cerr << "getRangeAtomicValues invalid range: " << token << std::endl;
            return result;
        }

        const char* range = token.c_str();

        if (token.size() == 2)
        {
            handleExactPair(range, returnRange, result);
        }
        else if (token.size() == 3)
        {
            handleThreeCharRange(range, returnRange, result);
        }
        else if (token.size() == 4)
        {
            handleFourCharRange(range, returnRange, result);
        }
    }

    return result;
}

void RangeParser::handleExactPair(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    std::string s2(1, range[1]);

    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 'd');
        result.push_back(s1 + 's' + s2 + 'h');
        result.push_back(s1 + 's' + s2 + 'c');
        result.push_back(s1 + 'd' + s2 + 'h');
        result.push_back(s1 + 'd' + s2 + 'c');
        result.push_back(s1 + 'c' + s2 + 'h');
    }
    else
    {
        result.push_back(s1 + s2);
    }
}

void RangeParser::handleThreeCharRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    std::string s2(1, range[1]);

    if (range[2] == 's')
    {
        handleSuitedRange(s1, s2, returnRange, result);
    }
    else if (range[2] == 'o')
    {
        handleOffsuitedRange(s1, s2, returnRange, result);
    }
    else if (range[0] == range[1] && range[2] == '+')
    {
        handlePairAndAboveRange(range[0], returnRange, result);
    }
}

void RangeParser::handleFourCharRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    std::string s1(1, range[0]);
    char c = range[1];

    if (range[2] == 'o' && range[3] == '+')
    {
        handleOffsuitedAndAboveRange(s1, c, returnRange, result);
    }
    else if (range[2] == 's' && range[3] == '+')
    {
        handleSuitedAndAboveRange(s1, c, returnRange, result);
    }
    else if (range[0] == range[2])
    {
        handleExactPairRange(range, returnRange, result);
    }
    else
    {
        result.push_back(range); // Real hand, not a pair
    }
}

void RangeParser::handleSuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                    std::vector<std::string>& result)
{
    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 's');
        result.push_back(s1 + 'd' + s2 + 'd');
        result.push_back(s1 + 'h' + s2 + 'h');
        result.push_back(s1 + 'c' + s2 + 'c');
    }
    else
    {
        result.push_back(s1 + s2 + 's');
    }
}

void RangeParser::handleOffsuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                       std::vector<std::string>& result)
{
    if (!returnRange)
    {
        result.push_back(s1 + 's' + s2 + 'd');
        result.push_back(s1 + 's' + s2 + 'c');
        result.push_back(s1 + 's' + s2 + 'h');
        result.push_back(s1 + 'd' + s2 + 's');
        result.push_back(s1 + 'd' + s2 + 'c');
        result.push_back(s1 + 'd' + s2 + 'h');
        result.push_back(s1 + 'h' + s2 + 'd');
        result.push_back(s1 + 'h' + s2 + 'c');
        result.push_back(s1 + 'h' + s2 + 's');
        result.push_back(s1 + 'c' + s2 + 'd');
        result.push_back(s1 + 'c' + s2 + 's');
        result.push_back(s1 + 'c' + s2 + 'h');
    }
    else
    {
        result.push_back(s1 + s2 + 'o');
    }
}

void RangeParser::handlePairAndAboveRange(char c, const bool returnRange, std::vector<std::string>& result)
{
    while (c != 'X')
    {
        std::string s(1, c);

        if (!returnRange)
        {
            result.push_back(s + 's' + s + 'd');
            result.push_back(s + 's' + s + 'c');
            result.push_back(s + 's' + s + 'h');
            result.push_back(s + 'd' + s + 'c');
            result.push_back(s + 'd' + s + 'h');
            result.push_back(s + 'h' + s + 'c');
        }
        else
        {
            result.push_back(s + s);
        }

        c = incrementCardValue(c);
    }
}

void RangeParser::handleOffsuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                               std::vector<std::string>& result)
{
    while (c != s1[0])
    {
        std::string s2(1, c);

        if (!returnRange)
        {
            result.push_back(s1 + 's' + s2 + 'd');
            result.push_back(s1 + 's' + s2 + 'c');
            result.push_back(s1 + 's' + s2 + 'h');
            result.push_back(s1 + 'd' + s2 + 's');
            result.push_back(s1 + 'd' + s2 + 'c');
            result.push_back(s1 + 'd' + s2 + 'h');
            result.push_back(s1 + 'h' + s2 + 'd');
            result.push_back(s1 + 'h' + s2 + 'c');
            result.push_back(s1 + 'h' + s2 + 's');
            result.push_back(s1 + 'c' + s2 + 'd');
            result.push_back(s1 + 'c' + s2 + 's');
            result.push_back(s1 + 'c' + s2 + 'h');
        }
        else
        {
            result.push_back(s1 + s2 + 'o');
        }

        c = incrementCardValue(c);
    }
}

void RangeParser::handleSuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                            std::vector<std::string>& result)
{
    while (c != s1[0])
    {
        std::string s2(1, c);

        if (!returnRange)
        {
            result.push_back(s1 + 's' + s2 + 's');
            result.push_back(s1 + 'd' + s2 + 'd');
            result.push_back(s1 + 'h' + s2 + 'h');
            result.push_back(s1 + 'c' + s2 + 'c');
        }
        else
        {
            result.push_back(s1 + s2 + 's');
        }

        c = incrementCardValue(c);
    }
}

void RangeParser::handleExactPairRange(const char* range, const bool returnRange, std::vector<std::string>& result)
{
    if (returnRange)
    {
        std::string s(1, range[0]);
        s += range[0];
        result.push_back(s);
    }
    else
    {
        result.push_back(range);
    }
}

char RangeParser::incrementCardValue(char c)
{

    switch (c)
    {
    case '2':
        return '3';
    case '3':
        return '4';
    case '4':
        return '5';
    case '5':
        return '6';
    case '6':
        return '7';
    case '7':
        return '8';
    case '8':
        return '9';
    case '9':
        return 'T';
    case 'T':
        return 'J';
    case 'J':
        return 'Q';
    case 'Q':
        return 'K';
    case 'K':
        return 'A';
    default:
        return 'X';
    }
}
} // namespace pkt::core::player