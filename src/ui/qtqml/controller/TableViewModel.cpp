// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "TableViewModel.h"

namespace pkt::ui::qtqml::controller
{

TableViewModel::TableViewModel(QObject* parent) : QObject(parent)
{
}

// Property setters with change notification

void TableViewModel::setGameActive(bool active)
{
    if (m_gameActive != active)
    {
        m_gameActive = active;
        emit gameActiveChanged();
    }
}

void TableViewModel::setGameStateText(const QString& text)
{
    if (m_gameStateText != text)
    {
        m_gameStateText = text;
        emit gameStateTextChanged();
    }
}

void TableViewModel::setPotAmount(int amount)
{
    if (m_potAmount != amount)
    {
        m_potAmount = amount;
        emit potAmountChanged();
    }
}

void TableViewModel::setBoardCards(const QVariantList& cards)
{
    if (m_boardCards != cards)
    {
        m_boardCards = cards;
        emit boardCardsChanged();
    }
}

void TableViewModel::setPlayers(const QVariantList& players)
{
    if (m_players != players)
    {
        m_players = players;
        emit playersChanged();
    }
}

void TableViewModel::setCurrentPlayerId(int id)
{
    if (m_currentPlayerId != id)
    {
        m_currentPlayerId = id;
        emit currentPlayerIdChanged();
    }
}

void TableViewModel::setHumanPlayerId(int id)
{
    if (m_humanPlayerId != id)
    {
        m_humanPlayerId = id;
        emit humanPlayerIdChanged();
    }
}

void TableViewModel::setAwaitingHumanInput(bool waiting)
{
    if (m_awaitingHumanInput != waiting)
    {
        m_awaitingHumanInput = waiting;
        emit awaitingHumanInputChanged();
    }
}

void TableViewModel::setValidActions(const QStringList& actions)
{
    if (m_validActions != actions)
    {
        m_validActions = actions;
        emit validActionsChanged();
    }
}

void TableViewModel::setMinBet(int amount)
{
    if (m_minBet != amount)
    {
        m_minBet = amount;
        emit minBetChanged();
    }
}

void TableViewModel::setMaxBet(int amount)
{
    if (m_maxBet != amount)
    {
        m_maxBet = amount;
        emit maxBetChanged();
    }
}

void TableViewModel::setCurrentBet(int amount)
{
    if (m_currentBet != amount)
    {
        m_currentBet = amount;
        emit currentBetChanged();
    }
}

void TableViewModel::setHandResult(const QString& result)
{
    if (m_handResult != result)
    {
        m_handResult = result;
        emit handResultChanged();
    }
}

void TableViewModel::setWinners(const QVariantList& winners)
{
    if (m_winners != winners)
    {
        m_winners = winners;
        emit winnersChanged();
    }
}

// Helper methods

void TableViewModel::updatePlayer(int playerId, const QVariantMap& playerData)
{
    int index = findPlayerIndex(playerId);
    if (index >= 0 && index < m_players.size())
    {
        QVariantMap existing = m_players[index].toMap();
        // Merge new data into existing
        for (auto it = playerData.begin(); it != playerData.end(); ++it)
        {
            existing[it.key()] = it.value();
        }
        m_players[index] = existing;
        emit playersChanged();
    }
}

void TableViewModel::updatePlayerCards(int playerId, const QString& card1, const QString& card2)
{
    int index = findPlayerIndex(playerId);
    if (index >= 0 && index < m_players.size())
    {
        QVariantMap player = m_players[index].toMap();
        player["card1"] = card1;
        player["card2"] = card2;
        player["hasCards"] = true;
        m_players[index] = player;
        emit playersChanged();
    }
}

void TableViewModel::updatePlayerChips(int playerId, int chips)
{
    int index = findPlayerIndex(playerId);
    if (index >= 0 && index < m_players.size())
    {
        QVariantMap player = m_players[index].toMap();
        player["chips"] = chips;
        m_players[index] = player;
        emit playersChanged();
    }
}

void TableViewModel::updatePlayerAction(int playerId, const QString& action, int amount)
{
    int index = findPlayerIndex(playerId);
    if (index >= 0 && index < m_players.size())
    {
        QVariantMap player = m_players[index].toMap();
        player["lastAction"] = action;
        player["lastBet"] = amount;
        m_players[index] = player;
        emit playersChanged();
    }
}

void TableViewModel::updatePlayerFolded(int playerId, bool folded)
{
    int index = findPlayerIndex(playerId);
    if (index >= 0 && index < m_players.size())
    {
        QVariantMap player = m_players[index].toMap();
        player["folded"] = folded;
        m_players[index] = player;
        emit playersChanged();
    }
}

int TableViewModel::findPlayerIndex(int playerId) const
{
    for (int i = 0; i < m_players.size(); ++i)
    {
        QVariantMap player = m_players[i].toMap();
        if (player["id"].toInt() == playerId)
        {
            return i;
        }
    }
    return -1;
}

void TableViewModel::reset()
{
    setGameActive(false);
    setGameStateText("");
    setPotAmount(0);
    setBoardCards(QVariantList());
    setPlayers(QVariantList());
    setCurrentPlayerId(-1);
    setHumanPlayerId(-1);
    setAwaitingHumanInput(false);
    setValidActions(QStringList());
    setMinBet(0);
    setMaxBet(0);
    setCurrentBet(0);
    setHandResult("");
    setWinners(QVariantList());
}

} // namespace pkt::ui::qtqml::controller
