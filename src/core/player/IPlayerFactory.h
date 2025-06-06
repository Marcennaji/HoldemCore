#pragma once
#include <memory>
#include "Player.h"
#include "core/engine/model/TableProfile.h"

namespace pkt::core::player
{

class IPlayerFactory
{
  public:
    virtual ~IPlayerFactory() = default;

    virtual std::shared_ptr<Player> createHumanPlayer(int id, const std::string& name, int startMoney) = 0;
    virtual std::shared_ptr<Player> createBotPlayer(int id, TableProfile profile) = 0;
};

} // namespace pkt::core::player
