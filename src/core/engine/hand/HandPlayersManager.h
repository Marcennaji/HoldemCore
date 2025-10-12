// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include "BettingActions.h"
#include "core/engine/model/PlayerAction.h"
#include "core/player/Player.h"
#include "typedefs.h"
#include "core/ports/Logger.h"
#include "core/engine/GameEvents.h"

namespace pkt::core
{

// Forward declarations
class Hand;

/**
 * @brief Manages player lists, positions, and player-related operations for poker hands.
 * 
 * This class extracts player management responsibilities from the Hand class,
 * providing focused player state management without coupling to game logic.
 * 
 * Responsibilities:
 * - Player list management (seats and acting players)
 * - Player filtering logic (insufficient cash, folded players)
 * - Player position tracking (dealer, blinds)
 * - Player lookup and validation
 * - Betting actions coordination
 */
class HandPlayersManager
{
  public:
    HandPlayersManager(pkt::core::player::PlayerList seats, 
                       pkt::core::player::PlayerList actingPlayers,
                       const GameEvents& events,
                       Logger& logger);
    ~HandPlayersManager() = default;

    // Player list access
    const pkt::core::player::PlayerList getSeatsList() const { return m_seatsList; }
    const pkt::core::player::PlayerList getActingPlayersList() const { return m_actingPlayersList; }
    pkt::core::player::PlayerList& getActingPlayersListMutable() { return m_actingPlayersList; }
    
    const pkt::core::player::PlayerList getPlayersInHandList() const
    {
        // Create a new list with players in hand (not folded, but might include allin players)
        auto playersInHand = std::make_shared<std::list<std::shared_ptr<pkt::core::player::Player>>>();

        for (const auto& player : *m_seatsList)
        {
            if (player->getLastAction().type != ActionType::Fold)
            {
                playersInHand->push_back(player);
            }
        }

        return playersInHand;
    }

    // Position management
    int getDealerPlayerId() const { return m_dealerPlayerId; }
    int getSmallBlindPlayerId() const { return m_smallBlindPlayerId; }
    int getBigBlindPlayerId() const { return m_bigBlindPlayerId; }
    
    void setDealerPlayerId(int playerId) { m_dealerPlayerId = playerId; }
    void setSmallBlindPlayerId(int playerId) { m_smallBlindPlayerId = playerId; }
    void setBigBlindPlayerId(int playerId) { m_bigBlindPlayerId = playerId; }

    // Betting actions access
    std::shared_ptr<BettingActions> getBettingActions() const { return m_bettingActions; }

    // Player management operations
    void filterPlayersWithInsufficientCash();
    void preparePlayersForNewHand(const Hand& hand);
    
    // Player validation
    std::shared_ptr<pkt::core::player::Player> validatePlayer(unsigned playerId) const;
    
  private:
    pkt::core::player::PlayerList m_seatsList;         // all players
    pkt::core::player::PlayerList m_actingPlayersList; // all players who have not folded and are not all in
    int m_dealerPlayerId{-1};
    int m_smallBlindPlayerId{-1};
    int m_bigBlindPlayerId{-1};
    std::shared_ptr<BettingActions> m_bettingActions;
    
    const GameEvents& m_events;
    Logger* m_logger;
};

} // namespace pkt::core