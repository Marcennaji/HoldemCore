#include "StrategyAssigner.h"
#include <random>
#include "LooseAggressiveBotStrategy.h"
#include "ManiacBotStrategy.h"
#include "TightAggressiveBotStrategy.h"
#include "UltraTightBotStrategy.h"

namespace pkt::core::player
{

StrategyAssigner::StrategyAssigner(TableProfile profile, int botCount, ILogger* logger)
    : myProfile(profile), maxPerType(botCount / 3), myLogger(logger)
{
}

std::unique_ptr<IBotStrategy> StrategyAssigner::chooseStrategyFor(int botIndex)
{
    int rand = std::rand() % 12;

    if (myProfile == TIGHT_AGRESSIVE_OPPONENTS)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<TightAggressiveBotStrategy>(myLogger);
        }
        else
        {
            return std::make_unique<UltraTightBotStrategy>(myLogger);
        }
    }

    if (myProfile == LARGE_AGRESSIVE_OPPONENTS)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<LooseAggressiveBotStrategy>(myLogger);
        }
        else
        {
            return std::make_unique<ManiacBotStrategy>(myLogger);
        }
    }

    // RANDOM_OPPONENTS profile
    if (rand < 3 && countManiac++ < maxPerType)
        return std::make_unique<ManiacBotStrategy>(myLogger);
    if (rand < 5 && countUltraTight++ < maxPerType)
        return std::make_unique<UltraTightBotStrategy>(myLogger);
    if (rand < 9 && countLoose++ < maxPerType)
        return std::make_unique<LooseAggressiveBotStrategy>(myLogger);
    if (countTight++ < maxPerType)
        return std::make_unique<TightAggressiveBotStrategy>(myLogger);

    // fallback
    return std::make_unique<UltraTightBotStrategy>(myLogger);
}

} // namespace pkt::core::player
