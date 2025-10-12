#include "StrategyAssigner.h"
#include <random>
#include "LooseAggressiveBotStrategy.h"
#include "ManiacBotStrategy.h"
#include "TightAggressiveBotStrategy.h"
#include "UltraTightBotStrategy.h"

namespace pkt::core::player
{

StrategyAssigner::StrategyAssigner(TableProfile profile, int botCount, 
                                   pkt::core::Logger& logger,
                                   pkt::core::Randomizer& randomizer) 
    : m_profile(profile), m_maxPerType(botCount / 3), m_logger(logger), m_randomizer(randomizer)
{
}

std::unique_ptr<BotStrategy> StrategyAssigner::chooseBotStrategyFor(int index)
{
    int rand = std::rand() % 12;

    if (m_profile == TightAgressiveOpponents)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<TightAggressiveBotStrategy>(m_logger, m_randomizer);
        }
        else
        {
            return std::make_unique<UltraTightBotStrategy>(m_logger, m_randomizer);
        }
    }

    if (m_profile == LargeAgressiveOpponents)
    {
        if (rand % 2 == 0)
        {
            return std::make_unique<LooseAggressiveBotStrategy>(m_logger, m_randomizer);
        }
        else
        {
            return std::make_unique<ManiacBotStrategy>(m_logger, m_randomizer);
        }
    }

    // RandomOpponents profile
    if (rand < 3 && m_countManiac++ < m_maxPerType)
    {
        return std::make_unique<ManiacBotStrategy>(m_logger, m_randomizer);
    }
    if (rand < 5 && m_countUltraTight++ < m_maxPerType)
    {
        return std::make_unique<UltraTightBotStrategy>(m_logger, m_randomizer);
    }
    if (rand < 9 && m_countLoose++ < m_maxPerType)
    {
        return std::make_unique<LooseAggressiveBotStrategy>(m_logger, m_randomizer);
    }
    if (m_countTight++ < m_maxPerType)
    {
        return std::make_unique<TightAggressiveBotStrategy>(m_logger, m_randomizer);
    }

    // fallback
    return std::make_unique<UltraTightBotStrategy>(m_logger, m_randomizer);
}

} // namespace pkt::core::player
