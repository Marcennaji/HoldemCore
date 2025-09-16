// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerListUtils.h"
#include "Player.h"
#include "core/engine/model/PlayerAction.h"
#include "core/services/GlobalServices.h"

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

void PlayerListUtils::updateActingPlayersList(PlayerList& myActingPlayersList)
{
    GlobalServices::instance().logger().verbose("Updating myActingPlayersList...");

    PlayerListIterator it, it1;

    for (it = myActingPlayersList->begin(); it != myActingPlayersList->end();)
    {
        GlobalServices::instance().logger().verbose("Checking player: " + (*it)->getName() +
                                                    ", action: " + actionTypeToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            GlobalServices::instance().logger().verbose(
                "Removing player: " + (*it)->getName() +
                " from myActingPlayersList due to action: " + actionTypeToString((*it)->getLastAction().type));

            it = myActingPlayersList->erase(it);

            if (!myActingPlayersList->empty())
            {
                GlobalServices::instance().logger().verbose(
                    "myActingPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myActingPlayersList->begin())
                {
                    GlobalServices::instance().logger().verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myActingPlayersList->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger().verbose("myActingPlayersList is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger().verbose("Player: " + (*it)->getName() +
                                                        " remains in myActingPlayersList. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger().verbose("Finished updating myActingPlayersList.");
}

} // namespace pkt::core::player
