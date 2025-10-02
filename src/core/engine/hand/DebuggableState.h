#pragma once

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/engine/game/Board.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{

class Hand;
class ServiceContainer;

class HandDebuggableState
{
  public:
    virtual ~HandDebuggableState() = default;

    virtual void logStateInfo(Hand& hand)
    {
    static std::shared_ptr<pkt::core::ServiceContainer> defaultServices =
      std::make_shared<pkt::core::AppServiceContainer>();
    auto services = defaultServices;
        logStateInfo(hand, services);
    }

    virtual void logStateInfo(Hand& hand, std::shared_ptr<pkt::core::ServiceContainer> services)
    {
        // Default implementation: log basic info
        std::string boardStr = hand.getStringBoard();
        int pot = hand.getBoard().getPot(hand);
        services->logger().info(""); // Empty line for spacing
        services->logger().info("Current State: " + gameStateToString(hand.getGameState()) + ", Board: " + boardStr +
                                ", Pot: " + std::to_string(pot));
    }
};

} // namespace pkt::core
