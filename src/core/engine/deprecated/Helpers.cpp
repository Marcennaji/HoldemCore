
#include "Helpers.h"
#include "core/engine/Exception.h"
#include "core/engine/model/PlayerPosition.h"
#include "core/player/deprecated/Player.h"
#include "core/services/GlobalServices.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <random>
#include <sstream>
#include <string.h>
#include <string>

namespace pkt::core
{
using namespace std;
using namespace pkt::core;
using namespace pkt::core::player;

void shufflePlayers(std::list<std::shared_ptr<Player>>& players, unsigned humanId)
{
    std::vector<std::shared_ptr<Player>> v(players.begin(), players.end());
    auto it = std::find_if(v.begin(), v.end(), [=](auto& p) { return p->getId() == humanId; });
    if (it != v.end())
    {
        std::swap(v.front(), *it);
    }

    std::mt19937 rng(std::time(nullptr));
    std::shuffle(v.begin() + 1, v.end(), rng);

    players.assign(v.begin(), v.end());
}

PlayerListIterator getPlayerListIteratorById(PlayerList list, unsigned id)
{
    return std::find_if(list->begin(), list->end(),
                        [id](const std::shared_ptr<Player>& p) { return p->getId() == id; });
}
std::shared_ptr<Player> getPlayerById(PlayerList list, unsigned id)
{
    for (auto i = list->begin(); i != list->end(); ++i)
    {
        if ((*i)->getId() == id)
            return *i;
    }
    return nullptr;
}

void updateActingPlayersList(PlayerList& myActingPlayersList)
{
    GlobalServices::instance().logger().verbose("Updating myActingPlayersList...");

    PlayerListIterator it, it1;

    for (it = myActingPlayersList->begin(); it != myActingPlayersList->end();)
    {
        GlobalServices::instance().logger().verbose("Checking player: " + (*it)->getName() +
                                                    ", action: " + playerActionToString((*it)->getLastAction().type));

        if ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
        {
            GlobalServices::instance().logger().verbose(
                "Removing player: " + (*it)->getName() +
                " from myActingPlayersList due to action: " + playerActionToString((*it)->getLastAction().type));

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
            ++it;
        }
    }

    GlobalServices::instance().logger().verbose("Finished updating myActingPlayersList.");
}

std::string getPositionLabel(PlayerPosition p)
{

    switch (p)
    {

    case UnderTheGun:
        return "UnderTheGun";
        break;
    case UnderTheGunPlusOne:
        return "UnderTheGun+1";
        break;
    case UnderTheGunPlusTwo:
        return "UnderTheGun+2";
        break;
    case Middle:
        return "Middle";
        break;
    case MiddlePlusOne:
        return "Middle+1";
        break;
    case Late:
        return "Late";
        break;
    case Cutoff:
        return "Cutoff";
        break;
    case Button:
        return "Button";
        break;
    case ButtonSmallBlind:
        return "Button / Small Blind";
        break;
    case SmallBlind:
        return "Small Blind";
        break;
    case BigBlind:
        return "Big Blind";
        break;
    default:
        return "unknown";
        break;
    }
}

PlayerListIterator findPlayerOrThrow(PlayerList seats, unsigned id)
{
    auto it = getPlayerListIteratorById(seats, id);
    if (it == seats->end())
    {
        string ids;
        for (auto i = seats->begin(); i != seats->end(); ++i)
            ids += " " + to_string((*i)->getId());
        GlobalServices::instance().logger().error("Couldn't find player with id " + to_string(id) +
                                                  " in the seats list. List contains following ids :" + ids);
        throw Exception(__FILE__, __LINE__, EngineError::ActingPlayerNotFound);
    }
    return it;
}

PlayerListIterator nextActivePlayer(PlayerList seats, PlayerListIterator it)
{
    ++it;
    if (it == seats->end())
        it = seats->begin();
    while ((*it)->getLastAction().type == ActionType::Fold || (*it)->getLastAction().type == ActionType::Allin)
    {
        ++it;
        if (it == seats->end())
            it = seats->begin();
    }
    return it;
}

} // namespace pkt::core
