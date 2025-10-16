// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include <core/engine/cards/Card.h>
#include <core/engine/model/GameState.h>
#include <core/engine/model/PlayerAction.h>

namespace pkt::core
{
class Session;
namespace player
{
class Player;
}
} // namespace pkt::core

namespace pkt::ui::qtqml::controller
{

/**
 * @brief View model that exposes poker game state to QML.
 *
 * This class acts as the bridge between C++ game logic and QML UI,
 * exposing all necessary game state as Q_PROPERTY for automatic QML binding.
 *
 * All properties use Qt types (QString, QVariantList, etc.) for seamless QML integration.
 */
class TableViewModel : public QObject
{
    Q_OBJECT

    // Game state properties
    Q_PROPERTY(bool gameActive READ gameActive WRITE setGameActive NOTIFY gameActiveChanged)
    Q_PROPERTY(QString gameStateText READ gameStateText NOTIFY gameStateTextChanged)
    Q_PROPERTY(int potAmount READ potAmount NOTIFY potAmountChanged)
    Q_PROPERTY(QVariantList boardCards READ boardCards NOTIFY boardCardsChanged)

    // Player properties
    Q_PROPERTY(QVariantList players READ players NOTIFY playersChanged)
    Q_PROPERTY(int currentPlayerId READ currentPlayerId NOTIFY currentPlayerIdChanged)
    Q_PROPERTY(int humanPlayerId READ humanPlayerId NOTIFY humanPlayerIdChanged)

    // Human player interaction
    Q_PROPERTY(bool awaitingHumanInput READ awaitingHumanInput NOTIFY awaitingHumanInputChanged)
    Q_PROPERTY(QStringList validActions READ validActions NOTIFY validActionsChanged)
    Q_PROPERTY(int minBet READ minBet NOTIFY minBetChanged)
    Q_PROPERTY(int maxBet READ maxBet NOTIFY maxBetChanged)
    Q_PROPERTY(int currentBet READ currentBet NOTIFY currentBetChanged)

    // Hand results
    Q_PROPERTY(QString handResult READ handResult NOTIFY handResultChanged)
    Q_PROPERTY(QVariantList winners READ winners NOTIFY winnersChanged)

  public:
    explicit TableViewModel(QObject* parent = nullptr);

    // Property getters
    bool gameActive() const { return m_gameActive; }
    QString gameStateText() const { return m_gameStateText; }
    int potAmount() const { return m_potAmount; }
    QVariantList boardCards() const { return m_boardCards; }
    QVariantList players() const { return m_players; }
    int currentPlayerId() const { return m_currentPlayerId; }
    int humanPlayerId() const { return m_humanPlayerId; }
    bool awaitingHumanInput() const { return m_awaitingHumanInput; }
    QStringList validActions() const { return m_validActions; }
    int minBet() const { return m_minBet; }
    int maxBet() const { return m_maxBet; }
    int currentBet() const { return m_currentBet; }
    QString handResult() const { return m_handResult; }
    QVariantList winners() const { return m_winners; }

    // Property setters
    void setGameActive(bool active);
    void setGameStateText(const QString& text);
    void setPotAmount(int amount);
    void setBoardCards(const QVariantList& cards);
    void setPlayers(const QVariantList& players);
    void setCurrentPlayerId(int id);
    void setHumanPlayerId(int id);
    void setAwaitingHumanInput(bool waiting);
    void setValidActions(const QStringList& actions);
    void setMinBet(int amount);
    void setMaxBet(int amount);
    void setCurrentBet(int amount);
    void setHandResult(const QString& result);
    void setWinners(const QVariantList& winners);

    // Helper methods for updating game state
    void updatePlayer(int playerId, const QVariantMap& playerData);
    void updatePlayerCards(int playerId, const QString& card1, const QString& card2);
    void updatePlayerChips(int playerId, int chips);
    void updatePlayerAction(int playerId, const QString& action, int amount);

    // Reset all state
    void reset();

  signals:
    void gameActiveChanged();
    void gameStateTextChanged();
    void potAmountChanged();
    void boardCardsChanged();
    void playersChanged();
    void currentPlayerIdChanged();
    void humanPlayerIdChanged();
    void awaitingHumanInputChanged();
    void validActionsChanged();
    void minBetChanged();
    void maxBetChanged();
    void currentBetChanged();
    void handResultChanged();
    void winnersChanged();

  private:
    // Helper to find player in list by ID
    int findPlayerIndex(int playerId) const;

    // Game state
    bool m_gameActive = false;
    QString m_gameStateText;
    int m_potAmount = 0;
    QVariantList m_boardCards;

    // Players
    QVariantList m_players;
    int m_currentPlayerId = -1;
    int m_humanPlayerId = -1;

    // Human interaction
    bool m_awaitingHumanInput = false;
    QStringList m_validActions;
    int m_minBet = 0;
    int m_maxBet = 0;
    int m_currentBet = 0;

    // Hand results
    QString m_handResult;
    QVariantList m_winners;
};

} // namespace pkt::ui::qtqml::controller
