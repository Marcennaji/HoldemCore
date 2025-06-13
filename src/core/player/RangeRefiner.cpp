#include "RangeRefiner.h"
#include "RangeManager.h"
#include "core/player/Helpers.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace pkt::core::player
{
using namespace std;

std::string RangeRefiner::deduceRange(const std::string originRanges, const std::string rangesToSubstract,
                                      const std::string board)
{
    std::istringstream oss(originRanges);
    std::string singleOriginRange;
    std::string newRange;

    while (getline(oss, singleOriginRange, ','))
    {
        if (singleOriginRange.empty())
            continue;

        std::vector<std::string> cardsInOriginRange = RangeManager::getRangeAtomicValues(singleOriginRange);
        bool keepOriginRange = true;

        for (const auto& originHand : cardsInOriginRange)
        {
            const std::string card1 = originHand.substr(0, 2);
            const std::string card2 = originHand.substr(2, 4);

            if (isCardsInRange(card1, card2, rangesToSubstract) || board.find(card1) != std::string::npos ||
                board.find(card2) != std::string::npos)
            {
                keepOriginRange = false;

                std::vector<std::string> atomicRangesInSingleOriginRange =
                    RangeManager::getRangeAtomicValues(singleOriginRange, true);

                for (const auto& atomicOriginRange : atomicRangesInSingleOriginRange)
                {
                    if (atomicOriginRange.size() == 4)
                    {
                        // is a real hand, like "KcJh"
                        processRealHands({atomicOriginRange}, rangesToSubstract, board, newRange);
                    }
                    else
                    {
                        // is a range like "AJo" (without the +), or "99".
                        // we need to get real cards from it

                        std::vector<std::string> handsInAtomicRange =
                            RangeManager::getRangeAtomicValues(atomicOriginRange);

                        if (card1.at(1) == card2.at(1))
                        {
                            processSuitedRanges(handsInAtomicRange, rangesToSubstract, board, atomicOriginRange,
                                                newRange);
                        }
                        else
                        {
                            processUnsuitedRanges(handsInAtomicRange, rangesToSubstract, board, atomicOriginRange,
                                                  newRange);
                        }
                    }
                }
            }
        }

        if (keepOriginRange)
        {
            newRange += "," + singleOriginRange;
        }
    }

    // Remove leading comma if present
    if (!newRange.empty() && newRange.front() == ',')
    {
        newRange = newRange.substr(1);
    }

    return newRange;
}

void RangeRefiner::processRealHands(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                    const std::string& board, std::string& newRange)
{
    for (const auto& hand : hands)
    {
        const std::string card1 = hand.substr(0, 2);
        const std::string card2 = hand.substr(2, 4);

        // Skip if the hand is in rangesToSubstract or conflicts with the board
        if (isCardsInRange(card1, card2, rangesToSubstract) || board.find(card1) != std::string::npos ||
            board.find(card2) != std::string::npos)
        {
            continue;
        }

        // Add the hand to newRange if not already present
        if (newRange.find(hand) == std::string::npos)
        {
            newRange += "," + hand;
        }
    }
}

void RangeRefiner::processSuitedRanges(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                       const std::string& board, const std::string& atomicOriginRange,
                                       std::string& newRange)
{
    std::string suitedRanges;
    int nbSuitedRanges = 0;

    for (const auto& hand : hands)
    {
        const std::string card1 = hand.substr(0, 2);
        const std::string card2 = hand.substr(2, 4);

        // Skip if the hand is in rangesToSubstract or conflicts with the board
        if (isCardsInRange(card1, card2, rangesToSubstract) || board.find(card1) != std::string::npos ||
            board.find(card2) != std::string::npos)
        {
            continue;
        }

        nbSuitedRanges++;
        suitedRanges += "," + hand;
    }

    if (nbSuitedRanges < 4)
    {
        newRange += suitedRanges; // Add individual hands
    }
    else
    {
        if (newRange.find(atomicOriginRange) == std::string::npos)
        {
            newRange += "," + atomicOriginRange; // Add the range for better readability
        }
    }
}
void RangeRefiner::processUnsuitedRanges(const std::vector<std::string>& hands, const std::string& rangesToSubstract,
                                         const std::string& board, const std::string& atomicOriginRange,
                                         std::string& newRange)
{
    for (const auto& hand : hands)
    {
        const std::string card1 = hand.substr(0, 2);
        const std::string card2 = hand.substr(2, 4);

        // Skip if the hand is in rangesToSubstract or conflicts with the board
        if (isCardsInRange(card1, card2, rangesToSubstract) || board.find(card1) != std::string::npos ||
            board.find(card2) != std::string::npos)
        {
            continue;
        }

        // Add the range if not already present
        if (newRange.find(atomicOriginRange) == std::string::npos)
        {
            newRange += "," + atomicOriginRange;
        }
    }
}

} // namespace pkt::core::player