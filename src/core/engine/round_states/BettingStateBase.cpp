
#include "BettingStateBase.h"
#include "core/engine/HandFsm.h"
#include "core/interfaces/hand/IPlayerAccess.h"
#include "core/player/PlayerFsm.h"

namespace pkt::core
{

bool BettingStateBase::isRoundComplete(const HandFsm& hand) const
{
    if (isOnlyOnePlayerRemaining(hand))
        return true;

    return haveAllPlayersCalledOrFolded(hand);
}
int BettingStateBase::getHighestSet() const
{
    return myHighestSet;
}

void BettingStateBase::updateHighestSet(int amount)
{
    if (amount > myHighestSet)
        myHighestSet = amount;
}

bool BettingStateBase::haveAllPlayersCalledOrFolded(const HandFsm& hand) const
{

    const auto& players = hand.getRunningPlayersList();
    /*const int lastAggressorIndex = hand.getLastAggressorIndex(); // must be stored/set during betting
    const int currentBet = hand.getHighestSetThisRound();

    for (const auto& player : players)
    {
        if (player->hasFolded() || player->isAllIn())
            continue;

        if (player->getStack() == 0)
            continue; // Just in case not marked as all-in

        if (player->getCurrentBet() < currentBet)
            return false; // hasn't called

        // Optionally check if they’ve acted after last aggressor
        // (can be tracked using hand.hasActedSinceLastRaise(player))
    }*/

    return true;
}

bool BettingStateBase::isOnlyOnePlayerRemaining(const HandFsm& hand) const
{
    return hand.getRunningPlayersList()->size() == 1;
}
} // namespace pkt::core
