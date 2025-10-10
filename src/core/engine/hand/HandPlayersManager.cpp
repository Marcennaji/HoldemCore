// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "HandPlayersManager.h"
#include "Hand.h"
#include "core/player/Helpers.h"

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandPlayersManager::HandPlayersManager(PlayerList seats, 
                                       PlayerList actingPlayers,
                                       const GameEvents& events,
                                       Logger& logger)
    : m_seatsList(seats), m_actingPlayersList(actingPlayers), 
      m_events(events), m_logger(&logger)
{
    m_bettingActions = make_shared<BettingActions>(m_seatsList, m_actingPlayersList);
}

void HandPlayersManager::filterPlayersWithInsufficientCash()
{
    // Remove players from acting players list who cannot afford minimum participation
    // Cards will only be dealt to players in the acting list, so this is sufficient
    auto it = m_actingPlayersList->begin();
    while (it != m_actingPlayersList->end())
    {
        const int playerCash = (*it)->getCash();
        
        // Players with zero cash cannot participate at all
        if (playerCash <= 0)
        {
            m_logger->info("Player " + (*it)->getName() + " (ID: " + 
                std::to_string((*it)->getId()) + ") auto-folded due to insufficient cash: " + 
                std::to_string(playerCash));
            
            // Create a fold action to notify the UI that this player is folded
            PlayerAction autoFoldAction;
            autoFoldAction.playerId = (*it)->getId();
            autoFoldAction.type = ActionType::Fold;
            autoFoldAction.amount = 0;
            
            // Fire the player action event to notify UI that this player folded
            if (m_events.onPlayerActed)
            {
                m_events.onPlayerActed(autoFoldAction);
            }
                
            it = m_actingPlayersList->erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Log the number of players remaining for the hand
    m_logger->debug("Hand will proceed with " + std::to_string(m_actingPlayersList->size()) + " players");
}

void HandPlayersManager::preparePlayersForNewHand(const Hand& hand)
{
    for (auto player = m_seatsList->begin(); player != m_seatsList->end(); ++player)
    {
        (*player)->resetForNewHand(hand);
    }
}

std::shared_ptr<pkt::core::player::Player> HandPlayersManager::validatePlayer(unsigned playerId) const
{
    return getPlayerById(m_actingPlayersList, playerId);
}

} // namespace pkt::core