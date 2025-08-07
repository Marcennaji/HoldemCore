// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "BotPlayerFsm.h"
#include <core/player/strategy/CurrentHandContext.h>
#include <core/player/strategy/IBotStrategy.h>
#include <core/services/GlobalServices.h>
#include "core/player/strategy/CurrentHandContext.h"

#include <sstream>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
namespace pkt::core::player
{

using namespace std;

BotPlayerFsm::BotPlayerFsm(const GameEvents& events, int id, std::string name, int cash)
    : PlayerFsm(events, id, name, cash)
{
}

BotPlayerFsm::~BotPlayerFsm() = default;

PlayerAction BotPlayerFsm::decidePreflopAction()
{
#if (False)
    updateCurrentHandContext(GameStatePreflop);

    PlayerAction action;
    action.playerId = getId();

    const int raiseAmount = myStrategy->preflopShouldRaise(*myCurrentHandContext);
    const bool shouldCall = myStrategy->preflopShouldCall(*myCurrentHandContext);
    const bool isBigBlind = myPosition == BB;
    const bool noRaises = currentHand->getPreflopRaisesNumber() == 0;

    myPreflopPotOdd = getPotOdd();

    if (raiseAmount > 0)
    {
        action.type = ActionType::Raise;
        action.amount = raiseAmount;
    }
    else if (noRaises && isBigBlind)
    {
        action.type = ActionType::Check;
    }
    else if (shouldCall)
    {
        action.type = ActionType::Call;
    }
    else
    {
        action.type = ActionType::Fold;
    }

    return action;
#else
    PlayerAction action;
    action.playerId = getId();
    action.type = ActionType::Fold;
    return action;
#endif
}

} // namespace pkt::core::player
