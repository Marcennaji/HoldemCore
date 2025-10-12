// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <assert.h>
#include <memory>

#include <core/engine/EngineDefs.h>
#include <core/session/Session.h>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QButtonGroup>

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;

/**
 * @brief Initial window for configuring and starting poker games.
 * 
 * This window allows users to set up game parameters such as number of players,
 * bot difficulty levels, blinds, starting chip counts, and other game settings
 * before launching the main poker table interface.
 * 
 * Dependencies are injected from the composition root (GuiAppController).
 */
class StartWindow : public QMainWindow
{
    Q_OBJECT
  public:
    StartWindow(PokerTableWindow* tableWindow, 
                pkt::core::Session* session,
                QWidget* parent = nullptr);

    ~StartWindow();

    void setSession(pkt::core::Session* session) { m_session = session; }
    pkt::core::Session* getSession()
    {
        assert(m_session != nullptr);
        return m_session;
    }

    bool eventFilter(QObject* obj, QEvent* event);

  signals:

  public slots:

    void startNewGame();

  private:
    PokerTableWindow* m_pokerTableWindow{nullptr};
    pkt::core::Session* m_session;

    void createInterface();
    void createTableSettings();
    void createOpponentsSettings(); 
    void createActionArea();
    void applyConsistentStyling();
    
    // UI Components
    QWidget* m_centralWidget;
    QGridLayout* m_mainLayout;
    QGroupBox* m_gameSettingsGroup;
    QGridLayout* m_gameSettingsLayout;
    
    // Table settings group
    QGroupBox* m_tableGroup;
    QLabel* m_playersLabel;
    QSpinBox* m_playersSpinBox;
    QLabel* m_cashLabel;
    QSpinBox* m_cashSpinBox;
    QLabel* m_blindLabel;
    QSpinBox* m_blindSpinBox;
    QLabel* m_speedLabel;
    QSpinBox* m_speedSpinBox;
    
    // Opponents profile group
    QGroupBox* m_opponentsGroup;
    QRadioButton* m_looseAggressiveRadio;
    QRadioButton* m_tightAggressiveRadio;
    QRadioButton* m_randomRadio;
    QButtonGroup* m_opponentsButtonGroup;
    
    // Action buttons
    QPushButton* m_startGameButton;
};

} // namespace pkt::ui::qtwidgets
