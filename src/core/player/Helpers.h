
// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

// Include the new focused modules
#include "core/engine/cards/Card.h"
#include "core/engine/cards/CardRangeAnalyzer.h"
#include "core/engine/probability/DrawProbabilityCalculator.h"
#include "core/player/PlayerListUtils.h"
#include "core/player/position/PositionManager.h"

#include "core/engine/model/GameState.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/model/PlayerPosition.h"
#include "typedefs.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

struct PostFlopAnalysisFlags;

namespace pkt::core::player
{

// Backward compatibility layer - delegates to the new modules
inline std::shared_ptr<Player> getPlayerById(PlayerList list, unsigned id)
{
    return PlayerListUtils::getPlayerById(list, id);
}

inline PlayerListIterator getPlayerListIteratorById(PlayerList list, unsigned id)
{
    return PlayerListUtils::getPlayerListIteratorById(list, id);
}

inline void updateActingPlayersList(PlayerList& list, pkt::core::Logger& logger)
{
    PlayerListUtils::updateActingPlayersList(list, logger);
}

inline bool isCardsInRange(const std::string& card1, const std::string& card2, const std::string& range)
{
    return pkt::core::cards::CardRangeAnalyzer::isCardsInRange(card1, card2, range);
}

inline bool isCardsInRange(const pkt::core::HoleCards& holeCards, const std::string& range)
{
    return pkt::core::cards::CardRangeAnalyzer::isCardsInRange(holeCards.card1.toString(), holeCards.card2.toString(),
                                                               range);
}

inline int getDrawingProbability(const PostFlopAnalysisFlags& state)
{
    return pkt::core::probability::DrawProbabilityCalculator::getDrawingProbability(state);
}

inline bool isDrawingProbOk(const PostFlopAnalysisFlags& state, int potOdd)
{
    return pkt::core::probability::DrawProbabilityCalculator::isDrawingProbOk(state, potOdd);
}

inline int getBoardCardsHigherThan(const std::string& stringBoard, const std::string& card)
{
    return pkt::core::cards::CardRangeAnalyzer::getBoardCardsHigherThan(stringBoard, card);
}

inline bool hasPosition(PlayerPosition m_pos, PlayerList actingPlayers)
{
    return pkt::core::player::position::PositionManager::hasPosition(m_pos, actingPlayers);
}

inline int playerDistanceCircularOffset(int fromId, int toId, const PlayerList& players)
{
    return pkt::core::player::position::PositionManager::playerDistanceCircularOffset(fromId, toId, players);
}

inline PlayerPosition computePlayerPositionFromOffset(int offset, int nbPlayers)
{
    return pkt::core::player::position::PositionManager::computePlayerPositionFromOffset(offset, nbPlayers);
}

} // namespace pkt::core::player