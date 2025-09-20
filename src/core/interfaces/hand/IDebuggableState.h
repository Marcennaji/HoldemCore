#pragma once

#include "core/engine/hand/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/interfaces/IBoard.h"
#include "core/services/ServiceContainer.h"

namespace pkt::core
{

class Hand;
class ServiceContainer;

class IDebuggableState
{
  public:
    virtual ~IDebuggableState() = default;

    virtual void logStateInfo(Hand& hand)
    {
        auto services = std::make_shared<pkt::core::AppServiceContainer>();
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
