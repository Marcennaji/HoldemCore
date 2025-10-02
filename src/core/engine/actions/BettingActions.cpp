
#include "BettingActions.h"
#include "../hand/Hand.h"
#include "core/player/Player.h"
#include "core/player/typedefs.h"

#include <algorithm>

namespace pkt::core
{
using namespace pkt::core::player;

BettingActions::BettingActions(PlayerList& seats, PlayerList& actingPlayers)
    : m_seatsList(seats), m_actingPlayersList(actingPlayers), m_preflop(GameState::Preflop, seats, actingPlayers),
      m_flop(GameState::Flop, seats, actingPlayers), m_turn(GameState::Turn, seats, actingPlayers),
      m_river(GameState::River, seats, actingPlayers)
{
}
int BettingActions::getMinRaise(int smallBlind) const
{
    if (!m_lastRaiserId.has_value())
    {
        // No raise yet: minimum raise is usually the big blind (preflop) or small blind
        return smallBlind;
    }

    // Compute previous raise amount
    int lastRaiserTotal = 0;
    for (auto player = m_actingPlayersList->begin(); player != m_actingPlayersList->end(); ++player)
    {
        if ((*player)->getId() == m_lastRaiserId.value())
        {
            PlayerAction lastAction = (*player)->getCurrentHandActions().getLastAction();
            assert(lastAction.type == ActionType::Raise || lastAction.type == ActionType::Allin);
            lastRaiserTotal = lastAction.amount;
            break;
        }
    }

    int prevHighest = m_roundHighestSet;
    int prevRaise = prevHighest - lastRaiserTotal;

    return prevRaise;
}

int BettingActions::getRoundHighestSet() const
{
    return m_roundHighestSet;
}

void BettingActions::updateRoundHighestSet(int amount)
{
    if (amount > m_roundHighestSet)
        m_roundHighestSet = amount;
}

void BettingActions::setLastActionPlayerId(int theValue)
{
    m_lastActionPlayerId = theValue;
    // m_board->setLastActionPlayerId(theValue);
}

std::vector<PlayerPosition> BettingActions::getRaisersPositions()
{

    std::vector<PlayerPosition> positions;

    for (auto itC = m_seatsList->begin(); itC != m_seatsList->end(); ++itC)
    { // note that all in players are not "running" any more

        if ((*itC)->getLastAction().type == ActionType::Raise || (*itC)->getLastAction().type == ActionType::Allin)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}

std::vector<PlayerPosition> BettingActions::getCallersPositions()
{

    std::vector<PlayerPosition> positions;

    for (auto itC = m_actingPlayersList->begin(); itC != m_actingPlayersList->end(); ++itC)
    {

        if ((*itC)->getLastAction().type == ActionType::Call)
        {
            positions.push_back((*itC)->getPosition());
        }
    }
    return positions;
}
int BettingActions::getLastRaiserId()
{

    auto lastRaiser = m_seatsList->end();

    auto players = m_seatsList;

    for (auto it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getLastAction().type == ActionType::Raise || (*it)->getLastAction().type == ActionType::Allin)
        {

            if (lastRaiser != m_seatsList->end())
            {
                if ((*lastRaiser)->getPosition() < (*it)->getPosition())
                {
                    lastRaiser = it;
                }
            }
            else
            {
                lastRaiser = it;
            }
        }
    }
    if (lastRaiser != m_seatsList->end())
    {
        return (*lastRaiser)->getId();
    }

    // if no raiser was found, look for the one who have bet (if any)

    for (auto it = players->begin(); it != players->end(); ++it)
    {

        if ((*it)->getLastAction().type == ActionType::Bet)
        {
            lastRaiser = it;
        }
    }
    if (lastRaiser != m_seatsList->end())
    {
        return (*lastRaiser)->getId();
    }
    else
    {
        return -1;
    }
}

void BettingActions::recordPlayerAction(GameState round, const pkt::core::PlayerAction& action)
{
    // Find or create entry for this round
    auto it = std::find_if(m_handActionHistory.begin(), m_handActionHistory.end(),
                           [round](const pkt::core::BettingRoundHistory& h) { return h.round == round; });

    if (it == m_handActionHistory.end())
    {
        m_handActionHistory.push_back({round, {{action.playerId, action.type}}});
    }
    else
    {
        it->actions.emplace_back(action.playerId, action.type);
    }
}

} // namespace pkt::core
