#include "StrategyAssigner.h"
#include <random>
#include "LooseAggressiveBotStrategy.h"
#include "ManiacBotStrategy.h"
#include "TightAggressiveBotStrategy.h"
#include "UltraTightBotStrategy.h"

namespace pkt::core::player
{

StrategyAssigner::StrategyAssigner(TableProfile profile, int botCount) : myProfile(profile), myMaxPerType(botCount / 3)
{
}

std::unique_ptr<BotStrategy> StrategyAssigner::chooseStrategyFor(int botIndex)
{
    int rand = std::rand() % 12;

    if (myProfile == TightAgressiveOpponents)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<TightAggressiveBotStrategy>();
        }
        else
        {
            return std::make_unique<UltraTightBotStrategy>();
        }
    }

    if (myProfile == LargeAgressiveOpponents)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<LooseAggressiveBotStrategy>();
        }
        else
        {
            return std::make_unique<ManiacBotStrategy>();
        }
    }

    // RandomOpponents profile
    if (rand < 3 && myCountManiac++ < myMaxPerType)
    {
        return std::make_unique<ManiacBotStrategy>();
    }
    if (rand < 5 && myCountUltraTight++ < myMaxPerType)
    {
        return std::make_unique<UltraTightBotStrategy>();
    }
    if (rand < 9 && myCountLoose++ < myMaxPerType)
    {
        return std::make_unique<LooseAggressiveBotStrategy>();
    }
    if (myCountTight++ < myMaxPerType)
    {
        return std::make_unique<TightAggressiveBotStrategy>();
    }

    // fallback
    return std::make_unique<UltraTightBotStrategy>();
}

} // namespace pkt::core::player
