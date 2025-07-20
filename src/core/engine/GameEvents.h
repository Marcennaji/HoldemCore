#pragma once
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"

#include <functional>
#include <string>
#include <vector>

namespace pkt::core
{

struct GameEvents
{
    std::function<void(int gameSpeed)> onGameInitialized;

    std::function<void(int)> onPotUpdated;
    std::function<void(unsigned playerId, int newChips)> onPlayerChipsUpdated;

    std::function<void()> onDealHoleCards;
    std::function<void(unsigned playerId)> onShowHoleCards;
    std::function<void(int bettingRoundId)> onDealCommunityCards;
    std::function<void()> onFlipHoleCardsAllIn;

    std::function<void(GameState)> onBettingRoundStarted;
    std::function<void(int playerId, ActionType)> onPlayerActed;
    std::function<void(int playerId, bool isActive)> onPlayerStatusChanged;

    std::function<void()> onAwaitingHumanInput;

    std::function<void()> onShowdownStarted;
    std::function<void()> onPauseHand;

    void clear()
    {
        *this = GameEvents(); // reset all handlers
    }
};
} // namespace pkt::core
