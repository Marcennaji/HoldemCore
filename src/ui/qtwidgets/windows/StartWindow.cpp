// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "StartWindow.h"

#include <ui/qtwidgets/windows/PokerTableWindow.h>

#include <core/engine/EngineDefs.h>
#include <core/engine/game/Game.h>
#include "core/services/ServiceContainer.h"

#include <core/engine/model/GameData.h>

using namespace std;
using namespace pkt::core;

namespace pkt::ui::qtwidgets
{

StartWindow::StartWindow(PokerTableWindow* tableWindow, Session* session, QWidget* parent)
    : QMainWindow(parent), myPokerTableWindow(tableWindow), mySession(session)
{
    setWindowTitle(QString(tr("HoldemCore %1").arg(HOLDEM_CORE__BETA_RELEASE_STRING)));
    setStatusBar(nullptr);
    setFixedSize(520, 400);
    
    // Ensure table window starts hidden until a game is started
    if (myPokerTableWindow) myPokerTableWindow->hide();

    createInterface();
    applyConsistentStyling();

    connect(m_startGameButton, &QPushButton::clicked, this, &StartWindow::startNewGame);

    // When the table window is closed, return to StartWindow
    if (myPokerTableWindow) {
        connect(myPokerTableWindow, &PokerTableWindow::windowClosed, this, [this]() {
            this->show();
        });
    }
    show();
}

StartWindow::StartWindow(PokerTableWindow* tableWindow, Session* session,
                         std::shared_ptr<pkt::core::ServiceContainer> services, QWidget* parent)
    : QMainWindow(parent), myPokerTableWindow(tableWindow), mySession(session), myServices(std::move(services))
{
    setWindowTitle(QString(tr("HoldemCore %1").arg(HOLDEM_CORE__BETA_RELEASE_STRING)));
    setStatusBar(nullptr);
    setFixedSize(520, 400);
    
    // Ensure table window starts hidden until a game is started
    if (myPokerTableWindow) myPokerTableWindow->hide();

    createInterface();
    applyConsistentStyling();

    connect(m_startGameButton, &QPushButton::clicked, this, &StartWindow::startNewGame);

    // When the table window is closed, return to StartWindow
    if (myPokerTableWindow) {
        connect(myPokerTableWindow, &PokerTableWindow::windowClosed, this, [this]() {
            this->show();
        });
    }
    show();
}

void StartWindow::createInterface()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QGridLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(16);
    
    // Main game settings group
    m_gameSettingsGroup = new QGroupBox("Game Settings", this);
    m_gameSettingsLayout = new QGridLayout(m_gameSettingsGroup);
    m_gameSettingsLayout->setContentsMargins(16, 24, 16, 16);
    m_gameSettingsLayout->setSpacing(16);
    
    // Table settings group
    m_tableGroup = new QGroupBox("Table", this);
    createTableSettings();
    
    // Opponents profile group  
    m_opponentsGroup = new QGroupBox("Opponents profile (initial state)", this);
    createOpponentsSettings();
    
    // Action area
    createActionArea();
    
    // Layout assembly
    m_gameSettingsLayout->addWidget(m_tableGroup, 0, 0, 1, 2);
    m_gameSettingsLayout->addWidget(m_opponentsGroup, 1, 0, 1, 2);
    
    // Center the start button
    auto actionLayout = new QHBoxLayout();
    actionLayout->addStretch();
    actionLayout->addWidget(m_startGameButton);
    actionLayout->addStretch();
    
    m_gameSettingsLayout->addLayout(actionLayout, 2, 0, 1, 2);
    
    m_mainLayout->addWidget(m_gameSettingsGroup, 0, 0);
}

void StartWindow::createTableSettings()
{
    auto layout = new QGridLayout(m_tableGroup);
    layout->setContentsMargins(16, 4, 16, 16);
    layout->setSpacing(12);
    
    // Number of players
    m_playersLabel = new QLabel("Nb. players:", this);
    m_playersSpinBox = new QSpinBox(this);
    m_playersSpinBox->setRange(2, 10);
    m_playersSpinBox->setValue(6);
    m_playersSpinBox->setFixedWidth(90);
    m_playersSpinBox->setFixedHeight(28);
    
    // Start cash
    m_cashLabel = new QLabel("Start Cash:", this);
    m_cashSpinBox = new QSpinBox(this);
    m_cashSpinBox->setPrefix("$");
    m_cashSpinBox->setRange(1000, 1000000);
    m_cashSpinBox->setSingleStep(50);
    m_cashSpinBox->setValue(2000);
    m_cashSpinBox->setFixedWidth(90);
    m_cashSpinBox->setFixedHeight(28);
    
    // Small blind
    m_blindLabel = new QLabel("Small blind:", this);
    m_blindSpinBox = new QSpinBox(this);
    m_blindSpinBox->setPrefix("$");
    m_blindSpinBox->setRange(1, 1000);
    m_blindSpinBox->setValue(10);
    m_blindSpinBox->setFixedWidth(90);
    m_blindSpinBox->setFixedHeight(28);
    
    // GUI speed
    m_speedLabel = new QLabel("Gui speed:", this);
    m_speedSpinBox = new QSpinBox(this);
    m_speedSpinBox->setRange(1, 10);
    m_speedSpinBox->setValue(8);
    m_speedSpinBox->setFixedWidth(90);
    m_speedSpinBox->setFixedHeight(28);
    
    // Layout arrangement with proper vertical alignment
    layout->addWidget(m_playersLabel, 0, 0, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_playersSpinBox, 0, 1, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_speedLabel, 0, 2, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_speedSpinBox, 0, 3, Qt::AlignVCenter | Qt::AlignLeft);
    
    layout->addWidget(m_cashLabel, 1, 0, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_cashSpinBox, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_blindLabel, 1, 2, Qt::AlignVCenter | Qt::AlignLeft);
    layout->addWidget(m_blindSpinBox, 1, 3, Qt::AlignVCenter | Qt::AlignLeft);
    
    // Set consistent row heights and column widths
    layout->setRowMinimumHeight(0, 45);
    layout->setRowMinimumHeight(1, 45);
    
    // Set column stretches to ensure proper distribution
    layout->setColumnStretch(0, 1);  // Label columns
    layout->setColumnStretch(1, 1);  // SpinBox columns  
    layout->setColumnStretch(2, 1);  // Label columns
    layout->setColumnStretch(3, 1);  // SpinBox columns
    
    // Add some horizontal spacing between columns
    layout->setColumnMinimumWidth(0, 130);
    layout->setColumnMinimumWidth(1, 100);
    layout->setColumnMinimumWidth(2, 110);
    layout->setColumnMinimumWidth(3, 100);
}

void StartWindow::createOpponentsSettings()
{
    auto layout = new QVBoxLayout(m_opponentsGroup);
    layout->setContentsMargins(16, 4, 16, 20);
    layout->setSpacing(12);
    
    m_opponentsButtonGroup = new QButtonGroup(this);
    
    m_randomRadio = new QRadioButton("Opponents randomly chosen", this);
    m_looseAggressiveRadio = new QRadioButton("Mostly loose-aggressive opponents", this);
    m_tightAggressiveRadio = new QRadioButton("Mostly tight-aggressive opponents", this);
    
    m_randomRadio->setChecked(true);
    
    m_opponentsButtonGroup->addButton(m_randomRadio, 2);
    m_opponentsButtonGroup->addButton(m_looseAggressiveRadio, 0);
    m_opponentsButtonGroup->addButton(m_tightAggressiveRadio, 1);
    
    layout->addWidget(m_randomRadio);
    layout->addWidget(m_looseAggressiveRadio);
    layout->addWidget(m_tightAggressiveRadio);
    
    // Remove the bottom stretch to move content up
    // layout->addStretch(); - Removed to move content up
}

void StartWindow::createActionArea()
{
    m_startGameButton = new QPushButton("Start game", this);
    m_startGameButton->setMinimumSize(140, 36);
    m_startGameButton->setMaximumSize(140, 36);
}

void StartWindow::applyConsistentStyling()
{
    // Apply window background and overall styling consistent with PokerTableWindow
    setStyleSheet(
        "QMainWindow {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #f0f4f7, stop: 0.4 #e8ecf0, stop: 1 #dde3e9);"
        "}"
        
        "QGroupBox {"
        "  background-color: #f8fafc;"
        "  border: 1.5px solid #b0b0b0;"
        "  border-radius: 8px;"
        "  margin-top: 12px;"
        "  color: #333;"
        "  font-weight: normal;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px 0 4px;"
        "  font-weight: 600;"
        "  font-size: 13px;"
        "}"
        
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffffff, stop: 1 #f1f3f5);"
        "  border: 2px solid #dee2e6;"
        "  border-radius: 10px;"
        "  padding: 12px 18px;"
        "  color: #2c3e50;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  min-height: 24px;"
        "}"
        "QPushButton:hover {"
        "  background: #f8f9fa;"
        "  border-color: #adb5bd;"
        "}"
        "QPushButton:pressed {"
        "  background: #e9ecef;"
        "}"
        "QPushButton:disabled {"
        "  background: #f1f3f5;"
        "  color: #a1a1a1;"
        "  border-color: #e9ecef;"
        "}"
        
        "QSpinBox, QDoubleSpinBox {"
        "  background: #ffffff;"
        "  border: 1px solid #ced4da;"
        "  border-radius: 6px;"
        "  padding: 6px 8px;"
        "  color: #495057;"
        "  font-size: 13px;"
        "  min-height: 16px;"
        "  max-height: 28px;"
        "}"
        "QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border-color: #80bdff;"
        "  background: #fff;"
        "}"
        
        "QRadioButton {"
        "  color: #495057;"
        "  font-size: 13px;"
        "  spacing: 8px;"
        "  padding: 4px 0px;"
        "  min-height: 20px;"
        "}"
        "QRadioButton::indicator {"
        "  width: 14px;"
        "  height: 14px;"
        "}"
        "QRadioButton::indicator:unchecked {"
        "  border: 2px solid #ced4da;"
        "  border-radius: 7px;"
        "  background: #ffffff;"
        "}"
        "QRadioButton::indicator:checked {"
        "  border: 2px solid #495057;"
        "  border-radius: 7px;"
        "  background: #ffffff;"
        "}"
        
        "QCheckBox {"
        "  color: #495057;"
        "  font-size: 13px;"
        "  spacing: 8px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 14px;"
        "  height: 14px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "  border: 2px solid #ced4da;"
        "  border-radius: 3px;"
        "  background: #ffffff;"
        "}"
        "QCheckBox::indicator:checked {"
        "  border: 2px solid #28a745;"
        "  border-radius: 3px;"
        "  background: #28a745;"
        "}"
        
        "QLabel {"
        "  color: #495057;"
        "  font-size: 13px;"
        "}"
    );
    
    // Apply discreet, sober styling to the Start Game button
    m_startGameButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffffff, stop: 1 #f1f3f5);"
        "  border: 2px solid #6c757d;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  color: #495057;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  min-height: 20px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #f8f9fa, stop: 1 #e9ecef);"
        "  border-color: #495057;"
        "}"
        "QPushButton:pressed {"
        "  background: #e9ecef;"
        "  border-color: #343a40;"
        "}"
    );
}

void StartWindow::ensureServicesInitialized()
{
    if (!myServices)
    {
        myServices = std::make_shared<pkt::core::AppServiceContainer>();
    }
}

StartWindow::~StartWindow()
{
}

void StartWindow::startNewGame()
{
    this->hide();

    GameData gameData;

    gameData.maxNumberOfPlayers = m_playersSpinBox->value();
    gameData.startMoney = m_cashSpinBox->value();
    gameData.firstSmallBlind = m_blindSpinBox->value();

    if (m_looseAggressiveRadio->isChecked())
        gameData.tableProfile = LargeAgressiveOpponents;
    else if (m_tightAggressiveRadio->isChecked())
        gameData.tableProfile = TightAgressiveOpponents;
    else
        gameData.tableProfile = RandomOpponents;

    gameData.guiSpeed = m_speedSpinBox->value();

    StartData startData;
    int tmpDealerPos = 0;
    startData.numberOfPlayers = gameData.maxNumberOfPlayers;

    ensureServicesInitialized();
    myServices->randomizer().getRand(0, startData.numberOfPlayers - 1, 1, &tmpDealerPos);
    startData.startDealerPlayerId = static_cast<unsigned>(tmpDealerPos);

    // Initialize PokerTableWindow with the GameData BEFORE showing it
    if (myPokerTableWindow) {
        myPokerTableWindow->initializeWithGameData(gameData);
        myPokerTableWindow->show();
        myPokerTableWindow->raise();
        myPokerTableWindow->activateWindow();
    }

    mySession->startGame(gameData, startData);
}

bool StartWindow::eventFilter(QObject* obj, QEvent* event)
{
    // Use default behavior; allow StartWindow to close the app normally
    return QMainWindow::eventFilter(obj, event);
}
} // namespace pkt::ui::qtwidgets