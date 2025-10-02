#include "StrategyAssigner.h"
#include <random>
#include "LooseAggressiveBotStrategy.h"
#include "ManiacBotStrategy.h"
#include "TightAggressiveBotStrategy.h"
#include "UltraTightBotStrategy.h"

namespace pkt::core::player
{

StrategyAssigner::StrategyAssigner(TableProfile profile, int botCount) : m_profile(profile), m_maxPerType(botCount / 3)
{
}

std::unique_ptr<BotStrategy> StrategyAssigner::chooseBotStrategyFor(int index)
{
    int rand = std::rand() % 12;

    if (m_profile == TightAgressiveOpponents)
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

    if (m_profile == LargeAgressiveOpponents)
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
    if (rand < 3 && m_countManiac++ < m_maxPerType)
    {
        return std::make_unique<ManiacBotStrategy>();
    }
    if (rand < 5 && m_countUltraTight++ < m_maxPerType)
    {
        return std::make_unique<UltraTightBotStrategy>();
    }
    if (rand < 9 && m_countLoose++ < m_maxPerType)
    {
        return std::make_unique<LooseAggressiveBotStrategy>();
    }
    if (m_countTight++ < m_maxPerType)
    {
        return std::make_unique<TightAggressiveBotStrategy>();
    }

    // fallback
    return std::make_unique<UltraTightBotStrategy>();
}

} // namespace pkt::core::player
