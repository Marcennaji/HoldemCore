#pragma once

#include "core/engine/Hand.h"
#include "core/engine/model/GameState.h"
#include "core/interfaces/IBoard.h"
#include "core/services/GlobalServices.h"

namespace pkt::core
{

class Hand;

class IDebuggableState
{
  public:
    virtual ~IDebuggableState() = default;

    virtual void logStateInfo(Hand& hand)
    {
        // Default implementation: log basic info
        std::string boardStr = hand.getStringBoard();
        int pot = hand.getBoard().getPot(hand);
        GlobalServices::instance().logger().info(""); // Empty line for spacing
        GlobalServices::instance().logger().info("Current State: " + gameStateToString(hand.getGameState()) +
                                                 ", Board: " + boardStr + ", Pot: " + std::to_string(pot));
    }
};

} // namespace pkt::core
