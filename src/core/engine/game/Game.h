// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <core/player/typedefs.h>
#include "EngineDefs.h"
#include "model/GameData.h"
#include "model/PlayerAction.h"
#include "model/StartData.h"

#include <list>
#include <memory>
#include <string>

namespace pkt::core
{

struct GameEvents;
class Hand;
class Board;
class EngineFactory;

/**
 * @brief Main game controller that manages poker game flow and state.
 * 
 * This class orchestrates a complete poker game session, managing players,
 * hands, dealer rotation, and overall game progression. It coordinates between
 * the engine factory, game events, and individual hand management.
 */
class Game
{
  public:
    Game(const GameEvents& events, EngineFactory& factory, std::shared_ptr<Board> board,
         pkt::core::player::PlayerList seatsList, unsigned dealerId, const GameData& gameData,
         const StartData& startData);

    ~Game() = default;

    void startNewHand();
    void handlePlayerAction(const PlayerAction& action);

  private:
    void findNextDealer();

  private:
    EngineFactory* m_engineFactory;
    const GameEvents& m_events;
    std::shared_ptr<Hand> m_currentHand;
    std::shared_ptr<Board> m_currentBoard;

    pkt::core::player::PlayerList m_seatsList;
    pkt::core::player::PlayerList m_actingPlayersList;

    unsigned m_dealerPlayerId{0};
    GameData m_gameData;
    StartData m_startData;
};

} // namespace pkt::core
