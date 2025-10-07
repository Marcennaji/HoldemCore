// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerListUtils.h"
#include "Player.h"
#include "core/engine/model/PlayerAction.h"
#include "core/interfaces/Logger.h"

#include <algorithm>

using namespace pkt::core;

namespace pkt::core::player
{

std::shared_ptr<Player> PlayerListUtils::getPlayerById(const PlayerList& list, unsigned id)
{
    for (auto i = list->begin(); i != list->end(); ++i)
    {
        if ((*i)->getId() == id)
            return *i;
    }
    return nullptr;
}

PlayerListIterator PlayerListUtils::getPlayerListIteratorById(PlayerList& list, unsigned id)
{
    return std::find_if(list->begin(), list->end(),
                        [id](const std::shared_ptr<Player>& p) { return p->getId() == id; });
}

void PlayerListUtils::updateActingPlayersList(PlayerList& m_actingPlayersList, pkt::core::Logger& logger)
{
    logger.verbose("Updating m_actingPlayersList...");

    PlayerListIterator it, it1;

    for (it = m_actingPlayersList->begin(); it != m_actingPlayersList->end();)
    {
        logger.verbose("Checking player: " + (*it)->getName() +
                       ", action: " + actionTypeToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            logger.verbose("Removing player: " + (*it)->getName() + " from m_actingPlayersList due to action: " +
                           actionTypeToString((*it)->getLastAction().type));

            it = m_actingPlayersList->erase(it);

            if (!m_actingPlayersList->empty())
            {
                logger.verbose("m_actingPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == m_actingPlayersList->begin())
                {
                    logger.verbose("Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = m_actingPlayersList->end();
                }
                --it1;
            }
            else
            {
                logger.verbose("m_actingPlayersList is now empty after removal.");
            }
        }
        else
        {
            logger.verbose("Player: " + (*it)->getName() +
                           " remains in m_actingPlayersList. Moving to the next player.");
            ++it;
        }
    }

    logger.verbose("Finished updating m_actingPlayersList.");
}

} // namespace pkt::core::player
