// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "PlayerListUtils.h"
#include "PlayerFsm.h"
#include "core/engine/model/PlayerAction.h"
#include "core/services/GlobalServices.h"

#include <algorithm>

using namespace pkt::core;

namespace pkt::core::player
{

std::shared_ptr<PlayerFsm> PlayerListUtils::getPlayerFsmById(const PlayerFsmList& list, unsigned id)
{
    for (auto i = list->begin(); i != list->end(); ++i)
    {
        if ((*i)->getId() == id)
            return *i;
    }
    return nullptr;
}

PlayerFsmListIterator PlayerListUtils::getPlayerFsmListIteratorById(PlayerFsmList& list, unsigned id)
{
    return std::find_if(list->begin(), list->end(),
                        [id](const std::shared_ptr<PlayerFsm>& p) { return p->getId() == id; });
}

void PlayerListUtils::updateActingPlayersListFsm(PlayerFsmList& myActingPlayersListFsm)
{
    GlobalServices::instance().logger().verbose("Updating myActingPlayersListFsm...");

    PlayerFsmListIterator it, it1;

    for (it = myActingPlayersListFsm->begin(); it != myActingPlayersListFsm->end();)
    {
        GlobalServices::instance().logger().verbose("Checking player: " + (*it)->getName() +
                                                    ", action: " + playerActionToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            GlobalServices::instance().logger().verbose(
                "Removing player: " + (*it)->getName() +
                " from myActingPlayersListFsm due to action: " + playerActionToString((*it)->getLastAction().type));

            it = myActingPlayersListFsm->erase(it);

            if (!myActingPlayersListFsm->empty())
            {
                GlobalServices::instance().logger().verbose(
                    "myActingPlayersListFsm is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myActingPlayersListFsm->begin())
                {
                    GlobalServices::instance().logger().verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myActingPlayersListFsm->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger().verbose("myActingPlayersListFsm is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger().verbose(
                "Player: " + (*it)->getName() + " remains in myActingPlayersListFsm. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger().verbose("Finished updating myActingPlayersListFsm.");
}

} // namespace pkt::core::player
