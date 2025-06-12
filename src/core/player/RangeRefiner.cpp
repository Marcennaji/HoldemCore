#include "RangeRefiner.h"
#include "RangeManager.h"
#include "core/player/Helpers.h"

#include <iostream>
#include <sstream>

namespace pkt::core::player
{
using namespace std;

std::string RangeRefiner::substractRange(const std::string originRanges, const std::string rangesToSubstract,
                                         const std::string board)
{

    std::istringstream oss(originRanges);
    std::string singleOriginRange;
    string newRange;

    while (getline(oss, singleOriginRange, ','))
    {

        if (singleOriginRange.size() == 0)
            continue;

        // singleOriginRange may contain 1 range of any type, i.e : "AJo+"  "99"  "22+"  "87s" , or even a real hand
        // like "As3c"

        vector<std::string> cardsInOriginRange = RangeManager::getRangeAtomicValues(
            singleOriginRange); // split this range (if needed) into real cards, like ",AhJc,AsQc,....."

        bool keepOriginRange = true;

        for (vector<std::string>::const_iterator originHand = cardsInOriginRange.begin();
             originHand != cardsInOriginRange.end(); originHand++)
        {

            const string originCard1 = (*originHand).substr(0, 2);
            const string originCard2 = (*originHand).substr(2, 4);

            if (isCardsInRange(originCard1, originCard2, rangesToSubstract) ||
                (board.find(originCard1) != string::npos || board.find(originCard2) != string::npos))
            {

                // if the hand is in the ranges to substract, or if one of the cards is on the board, we must remove it

                keepOriginRange = false; // at least one hand must be substracted from the singleOriginRange, so we must
                                         // replace this range by an other (smaller) range

                // check if this hand has been previously included in the new range, when processing an other range
                std::string::size_type pos = newRange.find(*originHand);
                if (pos != std::string::npos)
                {
#ifdef LOG_POKER_EXEC
                    std::cout << "removing previously included hand";
#endif
                    newRange = newRange.erase(pos, 4);
#ifdef LOG_POKER_EXEC
                    std::cout << "...new range is now " << newRange << endl;
#endif
                    continue;
                }

                // cout << endl << "must remove " << originCard1 << originCard2 << endl;

                vector<std::string> atomicRangesInSingleOriginRange =
                    RangeManager::getRangeAtomicValues(singleOriginRange, true);
                // atomicRangesInSingleOriginRange will now contain the singleOriginRange ranges, but converted to
                // remove the + signs. It may also contain real hands, like 5h4h. purpose : we will try to keep as few
                // "real hands" as possible, for better display readability via the GUI

                for (vector<std::string>::const_iterator atomicOriginRange = atomicRangesInSingleOriginRange.begin();
                     atomicOriginRange != atomicRangesInSingleOriginRange.end(); atomicOriginRange++)
                {

                    // std::cout << "single origin atomic range is " << *atomicOriginRange << endl;

                    // if the "range" is in fact a real hand :
                    if (atomicOriginRange->size() == 4)
                    {

                        const string s1 = (*atomicOriginRange).substr(0, 2);
                        const string s2 = (*atomicOriginRange).substr(2, 4);
                        if (isCardsInRange(s1, s2, rangesToSubstract))
                            continue;
                        // delete hands that can't exist, given the board
                        if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                            continue;
                        if (newRange.find(*atomicOriginRange) == string::npos)
                            newRange += "," + (*atomicOriginRange); // don't put it twice in the new range

                        continue;
                    }

                    // process the real ranges like AQo AKo A5s 55 77 ....(i.e, atomic ranges, with no +)

                    if (originCard1.at(1) == originCard2.at(1))
                    {

                        // if we are processing a suited hand

                        string suitedRanges;
                        int nbSuitedRanges = 0;

                        vector<std::string> handsInAtomicRange = RangeManager::getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            if (newRange.find(*i) != string::npos)
                                continue; // don't put it twice in the new range

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            nbSuitedRanges++;
                            suitedRanges += "," + (*i); // we keep this hand
                        }

                        if (nbSuitedRanges < 4)
                        {
                            newRange += suitedRanges; // put the real hands, like AhJh AsJs
                        }
                        else
                        {
                            // put a range like AJs, for better readability, instead of putting "AhJh AdJd AcJc AsJs"
                            if (newRange.find(*atomicOriginRange) == string::npos)
                                newRange += "," + (*atomicOriginRange); // don't put it twice in the new range
                        }
                        // cout << "new range is now " << newRange << endl;
                    }

                    else
                    {

                        // unsuited hands, including pairs

                        vector<std::string> handsInAtomicRange = RangeManager::getRangeAtomicValues(*atomicOriginRange);

                        for (vector<std::string>::const_iterator i = handsInAtomicRange.begin();
                             i != handsInAtomicRange.end(); i++)
                        {

                            // std::cout << "hand in atomic range is " << *i << endl;

                            const string s1 = (*i).substr(0, 2);
                            const string s2 = (*i).substr(2, 4);

                            if (isCardsInRange(s1, s2, rangesToSubstract))
                                continue;

                            if (newRange.find(*atomicOriginRange) != string::npos)
                                continue; // don't put it twice in the new range

                            // delete hands that can't exist, given the board
                            if (board.find(s1) != string::npos || board.find(s2) != string::npos)
                                continue;

                            // std::cout << "we keep it" << endl;
                            newRange += "," + (*atomicOriginRange); // we keep this range
                        }
                    }
                }
            }
        }

        if (keepOriginRange)
        { // all hands in the origin range are kept
            newRange += ",";
            newRange += singleOriginRange;
        }
    }
    // unsigned pos;
    // while ((pos = newRange.find(",,")) != string::npos)
    // newRange = newRange.replace(pos, 2, ",");

    // remove the leading comma if it exists
    if (newRange.size() > 0 && newRange.at(0) == ',')
        newRange = newRange.substr(1);

    return newRange;
}
} // namespace pkt::core::player