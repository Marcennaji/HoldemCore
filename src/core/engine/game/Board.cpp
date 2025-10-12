// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "Board.h"

#include "Exception.h"
#include "Pot.h"
#include "core/player/Player.h"
#include "hand/Hand.h"
#include "hand/HandEvaluator.h"
#include "core/engine/cards/CardUtilities.h"
#include "model/EngineError.h"

#include <algorithm>
#include <unordered_set>

// Helper methods for showdown reveal logic encapsulated in this translation unit
namespace {
    
    inline bool isNonFolded(const pkt::core::player::Player& p) {
        return p.getLastAction().type != pkt::core::ActionType::Fold;
    }

    inline int contribution(const pkt::core::player::Player& p) {
        return p.getCashAtHandStart() - p.getCash();
    }

    using Level = std::pair<int,int>; // (handRank, maxContributionAtLevel)

    // Update "levels" with a candidate (rank, contrib) and decide whether they must reveal per domain rules.
    // Returns true if candidate must reveal and mutates levels accordingly.
    bool processCandidateLevels(std::list<Level>& levels, int rank, int contrib) {
        if (levels.empty()) {
            levels.emplace_back(rank, contrib);
            return true;
        }
        for (auto it = levels.begin(); it != levels.end(); ++it) {
            const bool higherRank = rank > it->first;
            const bool equalRank = rank == it->first;

            if (higherRank) {
                auto next = it; ++next;
                if (next == levels.end()) {
                    // Higher than the last known level → reveal, create a new top level.
                    levels.emplace_back(rank, contrib);
                    return true;
                }
                // Otherwise, a higher rank exists before a stronger top level; continue scanning.
                continue;
            }

            if (equalRank) {
                auto next = it; ++next;
                // Reveal if this equals the current level and either there is no stricter next level,
                // or the contribution exceeds the next level's threshold.
                if (next == levels.end() || contrib > next->second) {
                    if (contrib > it->second) it->second = contrib; // raise current level threshold
                    return true;
                }
                return false;
            }

            // Lower rank: reveal only if contribution exceeds the current level's threshold;
            // insert a new level before current.
            if (contrib > it->second) {
                levels.insert(it, Level{rank, contrib});
                return true;
            }
            // Otherwise, no reveal; lower rank with insufficient contribution.
            return false;
        }
        return false;
    }

    // Advance iterator circularly by one within [begin,end)
    template <typename It, typename C>
    inline void advanceCircular(It& it, const C& cont) {
        ++it; if (it == cont->end()) it = cont->begin();
    }
}

namespace pkt::core
{
using namespace pkt::core::player;

Board::Board(unsigned dp, const GameEvents& events,
             Logger& logger, HandEvaluationEngine& handEvaluator)
    : m_dealerPlayerId(dp), m_events(events), m_logger(logger), m_handEvaluator(handEvaluator)
{
    m_boardCards.reset();
}

Board::~Board()
{
}

void Board::setSeatsList(PlayerList seats)
{
    m_seatsList = seats;
}
void Board::setActingPlayersList(PlayerList actingPlayers)
{
    m_actingPlayersList = actingPlayers;
}

void Board::distributePot(Hand& hand)
{
    int totalPot = 0;

    // Calculate total pot from all players at the table (including those who folded after contributing)
    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    // Recompute each player's rank with the final board to ensure correct showdown comparison
    const auto& bc = getBoardCards();
    if (bc.getNumCards() == 5)
    {
        m_logger.info(std::string("Showdown - final board: \"") + bc.toString() + "\"");
        // Only evaluate hands for players who actually participated in the hand
        for (auto& player : *hand.getActingPlayersList())
        {
            const auto& hc = player->getHoleCards();
            // In real games, hole cards are valid; recompute to ensure final ranks.
            // In unit tests that preset ranks, hole cards may be invalid; skip recompute to respect presets.
            if (hc.isValid())
            {
                // Build evaluator string strictly as: HOLE then BOARD (e.g., "Ah Ad 2c 7d 9h 4s 3c").
                std::string handStr = hc.toString() + std::string(" ") + bc.toString();
                // Extra diagnostic log to verify ordering at runtime.
                m_logger.debug(std::string("Recompute showdown with: \"") + handStr + "\"");
                player->setHandRanking(m_handEvaluator.rankHand(handStr.c_str()));
            }
            m_logger.info(
                std::string("Player ") + std::to_string(player->getId()) +
                " showdown hand: \"" + player->getHoleCards().toString() + "\" rank=" + std::to_string(player->getHandRanking())
            );
        }
    }

    // Use seats list for pot distribution (includes folded players who contributed)
    // but only evaluate hands for acting players (non-folded players who can win)
    Pot pot(totalPot, hand.getSeatsList(), m_dealerPlayerId, m_logger);
    pot.distribute();
    m_winners = pot.getWinners();

    if (m_events.onHandCompleted)
        m_events.onHandCompleted(m_winners, totalPot);
}

void Board::determineShowdownRevealOrder()
{
    m_showdownRevealOrder.clear();
    std::unordered_set<unsigned> seen; // maintain order uniqueness

    auto appendReveal = [&](unsigned id) {
        if (!seen.count(id)) { m_showdownRevealOrder.push_back(id); seen.insert(id); }
    };

    // All-in condition: everyone who didn't fold reveals, in seat order
    if (m_allInCondition)
    {
        for (auto it = m_seatsList->begin(); it != m_seatsList->end(); ++it) {
            if (isNonFolded(**it)) appendReveal((*it)->getId());
        }
        return;
    }

    // Find the last acting player who didn't fold; fallback to first non-folder
    PlayerListConstIterator lastIt = m_seatsList->end();
    for (auto it = m_seatsList->begin(); it != m_seatsList->end(); ++it) {
        if ((*it)->getId() == m_lastActionPlayerId && isNonFolded(**it)) { lastIt = it; break; }
    }
    if (lastIt == m_seatsList->end()) {
        for (auto it = m_seatsList->begin(); it != m_seatsList->end(); ++it) {
            if (isNonFolded(**it)) { lastIt = it; break; }
        }
    }
    if (lastIt == m_seatsList->end()) {
        // No players to reveal (all folded?)
        return;
    }

    // First reveal is the last actor
    appendReveal((*lastIt)->getId());

    // Initialize levels with the last actor's rank and contribution
    std::list<Level> levels;
    levels.emplace_back((*lastIt)->getHandRanking(), contribution(**lastIt));

    // Iterate circularly over the table, starting after last actor, up to N players
    auto it = lastIt; advanceCircular(it, m_seatsList);
    const unsigned n = static_cast<unsigned>(m_seatsList->size());
    for (unsigned k = 0; k < n; ++k) {
        if (isNonFolded(**it)) {
            const int rank = (*it)->getHandRanking();
            const int contrib = contribution(**it);
            if (processCandidateLevels(levels, rank, contrib)) {
                appendReveal((*it)->getId());
            }
        }
        advanceCircular(it, m_seatsList);
    }
}

void Board::setBoardCards(const BoardCards& boardCards)
{
    m_boardCards = boardCards;
}

const BoardCards& Board::getBoardCards() const
{
    return m_boardCards;
}

void Board::setAllInCondition(bool theValue)
{
    m_allInCondition = theValue;
}
void Board::setLastActionPlayerId(unsigned theValue)
{
    m_lastActionPlayerId = theValue;
}

std::list<unsigned> Board::getWinners() const
{
    return m_winners;
}
void Board::setWinners(const std::list<unsigned>& w)
{
    m_winners = w;
}

 
int Board::getPot(const Hand& hand) const
{
    int totalPot = 0;

    for (auto& player : *hand.getSeatsList())
    {
        totalPot += player->getCurrentHandActions().getHandTotalBetAmount();
    }
    return totalPot;
}
int Board::getSets(const Hand& hand) const
{
    int total = 0;
    GameState currentRound = hand.getGameState();

    for (auto& player : *hand.getSeatsList())
    {
        total += player->getCurrentHandActions().getRoundTotalBetAmount(currentRound);
    }
    return total;
}

std::string Board::getStringRepresentation() const
{
    if (m_boardCards.isPreflop() || !m_boardCards.isValid())
    {
        return ""; 
    }

    return " " + m_boardCards.toString();
}

} // namespace pkt::core
