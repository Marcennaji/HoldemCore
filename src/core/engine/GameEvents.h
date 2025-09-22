#pragma once
#include "cards/Card.h"
#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"

#include <functional>
#include <list>
#include <string>
#include <vector>

namespace pkt::core
{
namespace player
{
class HumanStrategy;
}

struct GameEvents
{
    std::function<void(int gameSpeed)> onGameInitialized;
    std::function<void(std::list<unsigned> winnerIds, int totalPot)> onHandCompleted;
    std::function<void(unsigned playerId, int newChips)> onPlayerChipsUpdated;
    std::function<void(GameState)> onBettingRoundStarted;
    std::function<void(int newPotAmount)> onPotUpdated;
    std::function<void(PlayerAction action)> onPlayerActed;
    std::function<void(unsigned playerId, std::vector<ActionType> validActions)> onAwaitingHumanInput;

    // Card dealing events
    std::function<void(unsigned playerId, HoleCards holeCards)> onHoleCardsDealt;
    std::function<void(BoardCards boardCards)> onBoardCardsDealt;

    // Error handling events
    std::function<void(unsigned playerId, PlayerAction invalidAction, std::string reason)> onInvalidPlayerAction;
    std::function<void(std::string errorMessage)> onEngineError;

    // Human strategy registration for direct GUI communication
    std::function<void(player::HumanStrategy*)> onHumanStrategyWaiting;
    
    // Event processing for responsive UI during strategy decisions (framework-agnostic)
    mutable std::function<void()> onProcessEvents;

    void clear()
    {
        *this = GameEvents(); // reset all handlers
    }
};
} // namespace pkt::core
