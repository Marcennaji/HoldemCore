#include "PokerTableWindow.h"
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QPixmap>
#include <QMessageBox>
#include <QPoint>
#include <QShowEvent>
#include <QDebug>
#include <cmath>

namespace pkt::ui::qtwidgets
{

// Action button styling constants
const QString btnStyle = 
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #8b4513, stop: 0.3 #6d3710, stop: 1 #5d2c0a);"
        "  border: 2px solid #654321;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  color: #ffd700;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 80px;"
        "}";

PokerTableWindow::PokerTableWindow(pkt::core::Session* session, QWidget* parent) 
    : QWidget(parent), m_session(session), m_maxPlayers(6), m_activePlayerId(-1), m_dealerPosition(-1) // Default to 6 players
{
    // Add null check for session
    if (!session) {
        // This could cause issues later
    }
    
    // Initialize player components vector for ALL players (including human)
    m_playerComponents.resize(m_maxPlayers); // Include human player
    
    setWindowTitle("Poker Table - HoldemCore");
    setMinimumSize(800, 600);
    
    // Add try-catch around UI setup to catch any crashes
    try {
        setupUi();
        connectSignals();
        
        // Demo: Show player state indicators (remove this in production)
        // Activate demo to showcase the new features:
        setDealerPosition(2);  // Set dealer at position 2
        setActivePlayer(0);    // Highlight human player as active
        
        // Community cards start hidden (will be shown when dealt)
        for (int i = 0; i < 5; ++i) {
            if (m_communityCards[i]) {
                m_communityCards[i]->setVisible(false);
                m_communityCards[i]->clear(); // Ensure no content initially
            }
        }
    } catch (...) {
        // Catch any exception during UI setup
    }
}

void PokerTableWindow::initializeWithGameData(const pkt::core::GameData& gameData)
{
    // Update max players and resize components if needed
    if (gameData.maxNumberOfPlayers != m_maxPlayers) {
        m_maxPlayers = gameData.maxNumberOfPlayers;
        
        // Clear existing player components
        for (auto& component : m_playerComponents) {
            if (component.playerGroup) {
                component.playerGroup->setParent(nullptr);
                delete component.playerGroup;
            }
        }
        
        // Resize and recreate ALL player components (including human player)
        m_playerComponents.clear();
        m_playerComponents.resize(m_maxPlayers); // Include human player
        
        // Recreate UI with new player count
        createPlayerAreas();
        
        // Position all players in circular layout
        positionPlayersInCircle();
    }
}

void PokerTableWindow::setupUi()
{
    // Create the main layout
    m_mainLayout = new QVBoxLayout(this);

    // Create all UI components properly
    createPlayerAreas();
    createCenterArea();
    createActionButtons();
    createBettingControls();
    
    // Create Next Hand button
    m_nextHandButton = new QPushButton("Next Hand", this);
    m_nextHandButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  border: 2px solid #45a049;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "  border: 2px solid #bbbbbb;"
        "}"
    );
    m_nextHandButton->setVisible(false);  // Hidden by default

    // Add all player widgets to the main widget (they will be positioned absolutely)
    for (auto& player : m_playerComponents) {
        if (player.playerGroup) {
            player.playerGroup->show();
        }
    }
    
    // Position all players in circular layout (will be called again in showEvent)
    positionPlayersInCircle();

    // Add stretch space to push action controls to bottom
    m_mainLayout->addStretch();
    
    // Create a horizontal layout for actions and betting controls side by side
    auto actionBettingLayout = new QHBoxLayout();
    actionBettingLayout->addWidget(m_actionGroup);
    actionBettingLayout->addWidget(m_bettingGroup);
    
    m_mainLayout->addLayout(actionBettingLayout);
    
    // Add Next Hand button (centered)
    auto nextHandLayout = new QHBoxLayout();
    nextHandLayout->addStretch();
    nextHandLayout->addWidget(m_nextHandButton);
    nextHandLayout->addStretch();
    m_mainLayout->addLayout(nextHandLayout);

    // Set layout and style
    this->setLayout(m_mainLayout);

    // Elegant light table background styling
    this->setStyleSheet(
        "PokerTableWindow {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #f8f9fa, stop: 0.5 #e9ecef, stop: 1 #ced4da);"
        "  border: 2px solid #6c757d;"
        "  border-radius: 12px;"
        "}"
        "QGroupBox {"
        "  border-radius: 6px;"
        "  font-weight: normal;"
        "}"
    );

    this->resize(1200, 800);

    this->show();
    this->raise();
    this->activateWindow();
}

void PokerTableWindow::createPlayerAreas()
{
    // Create UI components for ALL players (including human player at index 0)
    for (int i = 0; i < m_maxPlayers; ++i) {
        auto& player = m_playerComponents[i];
        
        // Mark if this is the human player (index 0)
        player.isHuman = (i == 0);
        
        QString playerName = player.isHuman ? "You" : QString("Bot %1").arg(i);
        player.playerGroup = new QGroupBox(playerName, this);
        
        // Unified styling for all players
        player.playerGroup->setStyleSheet(
            "QGroupBox {"
            "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
            "    stop: 0 #f8f9fa, stop: 0.5 #e9ecef, stop: 1 #dee2e6);"
            "  border: 1px solid #ced4da;"
            "  border-radius: 8px;"
            "  margin-top: 8px;"
            "  color: #495057;"
            "  font-weight: 500;"
            "  font-size: 12px;"
            "}"
            "QGroupBox:title {"
            "  subcontrol-origin: margin;"
            "  left: 8px;"
            "  padding: 0 4px 0 4px;"
            "}"
        );
        
        auto layout = new QVBoxLayout(player.playerGroup);

        player.nameLabel = new QLabel(player.isHuman ? "You" : "Empty", this);
        player.chipsLabel = new QLabel("$0", this);

        // Unified label styling for all players
        player.nameLabel->setStyleSheet(
            "QLabel {"
            "  color: #495057;"
            "  font-size: 13px;"
            "  font-weight: 500;"
            "  background-color: rgba(248, 249, 250, 200);"
            "  border: 1px solid #dee2e6;"
            "  border-radius: 4px;"
            "  padding: 3px;"
            "}"
        );
        player.chipsLabel->setStyleSheet(
            "QLabel {"
            "  color: #495057;"
            "  font-size: 12px;"
            "  font-weight: 500;"
            "  background-color: rgba(248, 249, 250, 180);"
            "  border: 1px solid #dee2e6;"
            "  border-radius: 4px;"
            "  padding: 3px;"
            "}"
        );

        // Hole cards in horizontal layout - larger for human player
        auto cardLayout = new QHBoxLayout();
        player.holeCard1 = new QLabel(this);
        player.holeCard2 = new QLabel(this);

        // Unified card styling and size for all players
        player.holeCard1->setFixedSize(55, 77); // Same size for all players
        player.holeCard2->setFixedSize(55, 77);
        QString cardStyle = 
            "QLabel {"
            "  border: 1px solid #ced4da;"
            "  border-radius: 6px;"
            "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
            "    stop: 0 #ffffff, stop: 1 #f8f9fa);"
            "  margin: 1px;"
            "}";
        player.holeCard1->setStyleSheet(cardStyle);
        player.holeCard2->setStyleSheet(cardStyle);
        
        player.holeCard1->setScaledContents(true);
        player.holeCard2->setScaledContents(true);

        player.holeCard1->setPixmap(getCardBackPixmap());
        player.holeCard2->setPixmap(getCardBackPixmap());

        cardLayout->addWidget(player.holeCard1);
        cardLayout->addWidget(player.holeCard2);

        // Create dealer button indicator
        player.dealerButton = new QLabel("D", this);
        player.dealerButton->setFixedSize(25, 25);
        player.dealerButton->setAlignment(Qt::AlignCenter);
        player.dealerButton->setStyleSheet(
            "QLabel {"
            "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
            "    stop: 0 #ffd700, stop: 1 #daa520);"
            "  border: 2px solid #8b6914;"
            "  border-radius: 12px;"
            "  color: #000000;"
            "  font-weight: bold;"
            "  font-size: 12px;"
            "}"
        );
        player.dealerButton->setVisible(false); // Initially hidden

        layout->addWidget(player.nameLabel);
        layout->addWidget(player.chipsLabel);
        layout->addLayout(cardLayout);
        layout->addWidget(player.dealerButton);

        player.playerGroup->setVisible(true);
        player.playerGroup->setFixedSize(120, 160); // Fixed size for circular positioning
    }
}

void PokerTableWindow::createActionButtons()
{
    m_actionGroup = new QGroupBox("Actions", this);
    m_actionGroup->setStyleSheet(
        "QGroupBox {"
        "  background-color: #f8fafc;"
        "  border: 1.5px solid #b0b0b0;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "  color: #333;"
        "  font-weight: normal;"
        "}"
        "QGroupBox:title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 3px 0 3px;"
        "}"
    );
    m_actionLayout = new QHBoxLayout(m_actionGroup);

    m_foldButton = new QPushButton("Fold", this);
    m_callButton = new QPushButton("Call", this);
    m_checkButton = new QPushButton("Check", this);
    m_betButton = new QPushButton("Bet", this);
    m_raiseButton = new QPushButton("Raise", this);
    m_allInButton = new QPushButton("All-In", this);

    // Poker-themed button styles
    QString btnStyle =
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #f0f0f0, stop: 0.3 #e0e0e0, stop: 1 #c0c0c0);"
        "  border: 2px solid #8b6914;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  color: #2c1810;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 80px;"
        "  box-shadow: 0 2px 4px rgba(0,0,0,0.3);"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffd700, stop: 0.3 #ffed4e, stop: 1 #e6c200);"
        "  border: 2px solid #b8860b;"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #e6c200, stop: 1 #ffd700);"
        "  box-shadow: inset 0 2px 4px rgba(0,0,0,0.3);"
        "}"
        "QPushButton:disabled {"
        "  background-color: #d0d0d0;"
        "  color: #888;"
        "  border: 2px solid #a0a0a0;"
        "  box-shadow: none;"
        "}";

    m_foldButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ff6b6b, stop: 0.3 #ff5252, stop: 1 #d32f2f);"
        "  border: 2px solid #8b0000;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ff8a80, stop: 1 #ff5252);"
        "}"
        "QPushButton:disabled {"
        "  background-color: #d0d0d0;"
        "  color: #888;"
        "  border: 2px solid #a0a0a0;"
        "}"
    );
    m_callButton->setStyleSheet(btnStyle);
    m_checkButton->setStyleSheet(btnStyle);
    m_betButton->setStyleSheet(btnStyle);
    m_raiseButton->setStyleSheet(btnStyle);
    m_allInButton->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #4caf50, stop: 0.3 #45a049, stop: 1 #2e7d32);"
        "  border: 2px solid #1b5e20;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  color: white;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #66bb6a, stop: 1 #4caf50);"
        "}"
        "QPushButton:disabled {"
        "  background-color: #d0d0d0;"
        "  color: #888;"
        "  border: 2px solid #a0a0a0;"
        "}"
    );

    m_actionLayout->addWidget(m_foldButton);
    m_actionLayout->addWidget(m_callButton);
    m_actionLayout->addWidget(m_checkButton);
    m_actionLayout->addWidget(m_betButton);
    m_actionLayout->addWidget(m_raiseButton);
    m_actionLayout->addWidget(m_allInButton);
}

void PokerTableWindow::createBettingControls()
{
    m_bettingGroup = new QGroupBox("Betting Amount", this);
    m_bettingGroup->setStyleSheet(
        "QGroupBox {"
        "  background-color: #f8fafc;"
        "  border: 1px solid #b0b0b0;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "  color: #333;"
        "  font-weight: normal;"
        "}"
        "QGroupBox:title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 3px 0 3px;"
        "}"
    );
    m_bettingLayout = new QHBoxLayout(m_bettingGroup);

    m_betAmountLabel = new QLabel("Amount: $0", this);
    m_betSlider = new QSlider(Qt::Horizontal, this);
    m_betSpinBox = new QSpinBox(this);

    m_betAmountLabel->setStyleSheet("color: #3a7ca5; font-size: 14px;");

    m_betSlider->setMinimum(0);
    m_betSlider->setMaximum(1000);
    m_betSlider->setValue(0);
    m_betSlider->setMinimumWidth(200);  // Ensure adequate width for usability

    m_betSpinBox->setMinimum(0);
    m_betSpinBox->setMaximum(10000);
    m_betSpinBox->setValue(0);
    m_betSpinBox->setMinimumWidth(80);  // Ensure adequate width

    m_bettingLayout->addWidget(m_betAmountLabel);
    m_bettingLayout->addWidget(m_betSlider);
    m_bettingLayout->addWidget(m_betSpinBox);

    // Set proportional sizing - action buttons take more space
    m_bettingGroup->setMaximumWidth(350);  // Limit betting area width
    m_bettingGroup->setEnabled(false);

    // Enhanced betting controls styling
    QString sliderStyle = 
        "QSlider::groove:horizontal {"
        "  border: 2px solid #8b4513;"
        "  height: 12px;"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #654321, stop: 1 #3e2723);"
        "  border-radius: 6px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffd700, stop: 1 #b8860b);"
        "  border: 2px solid #8b6914;"
        "  width: 24px;"
        "  margin: -8px 0;"
        "  border-radius: 12px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffed4a, stop: 1 #daa520);"
        "}";
    
    QString spinBoxStyle = 
        "QSpinBox {"
        "  background-color: #2c1810;"
        "  border: 2px solid #8b4513;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  color: #ffd700;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  min-width: 100px;"
        "}"
        "QSpinBox:focus {"
        "  border: 2px solid #ffd700;"
        "  background-color: #3d2415;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #8b4513, stop: 1 #654321);"
        "  border: 1px solid #654321;"
        "  width: 20px;"
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #a0522d, stop: 1 #8b4513);"
        "}";
    
    m_betSlider->setStyleSheet(sliderStyle);
    m_betSpinBox->setStyleSheet(spinBoxStyle);
}


void PokerTableWindow::connectSignals()
{
    // Connect button clicks to emit signals
    connect(m_foldButton, &QPushButton::clicked, this, &PokerTableWindow::foldClicked);
    connect(m_callButton, &QPushButton::clicked, this, &PokerTableWindow::callClicked);
    connect(m_checkButton, &QPushButton::clicked, this, &PokerTableWindow::checkClicked);
    connect(m_allInButton, &QPushButton::clicked, this, &PokerTableWindow::allInClicked);
    
    // Connect bet/raise buttons to onRaiseAction slot
    connect(m_betButton, &QPushButton::clicked, this, &PokerTableWindow::onRaiseAction);
    connect(m_raiseButton, &QPushButton::clicked, this, &PokerTableWindow::onRaiseAction);
    
    // Connect Next Hand button
    connect(m_nextHandButton, &QPushButton::clicked, this, &PokerTableWindow::onNextHandClicked);
    
    // Connect betting controls
    connect(m_betSlider, &QSlider::valueChanged, this, &PokerTableWindow::onBetAmountChanged);
    connect(m_betSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PokerTableWindow::onBetAmountChanged);
    
    // Enable all buttons initially for testing
    m_foldButton->setEnabled(true);
    m_callButton->setEnabled(true);
    m_checkButton->setEnabled(true);
    m_betButton->setEnabled(true);
    m_raiseButton->setEnabled(true);
    m_allInButton->setEnabled(true);
}

// Slot implementations
void PokerTableWindow::onBetAmountChanged(int amount)
{
    // Sync slider and spinbox
    if (sender() == m_betSlider) {
        m_betSpinBox->setValue(amount);
    } else if (sender() == m_betSpinBox) {
        m_betSlider->setValue(amount);
    }
    
    m_betAmountLabel->setText(QString("Amount: $%1").arg(amount));
}

void PokerTableWindow::onRaiseAction()
{
    int amount = m_betSpinBox->value();
    if (sender() == m_betButton) {
        emit betClicked(amount);
    } else {
        emit raiseClicked(amount);
    }
}

void PokerTableWindow::onNextHandClicked()
{
    // Hide the Next Hand button and emit signal to start next hand
    m_nextHandButton->setVisible(false);
    // m_statusLabel->setText("Starting next hand...");
    // m_statusLabel->setStyleSheet("color: #2d8659;");
    
    emit nextHandRequested();
}

// Card visualization helpers
QPixmap PokerTableWindow::getCardPixmap(const pkt::core::Card& card) const
{
    if (!card.isValid()) {
        return getCardBackPixmap();
    }
    
    QString imagePath = getCardImagePath(card.getIndex());
    QPixmap pixmap(imagePath);
    
    if (pixmap.isNull()) {
        // Fallback to text representation
        QPixmap fallback(50, 70);
        fallback.fill(Qt::white);
        return fallback;
    }
    
    return pixmap;
}

QPixmap PokerTableWindow::getCardBackPixmap() const
{
    QPixmap pixmap(":/cards/flipside.png");
    
    if (pixmap.isNull()) {
        // Fallback to colored rectangle if resource loading fails
        QPixmap fallback(50, 70);
        fallback.fill(Qt::blue);
        return fallback;
    }
    
    return pixmap;
}

QString PokerTableWindow::getCardImagePath(int cardIndex) const
{
    return QString(":/cards/%1.png").arg(cardIndex);
}

// Public interface implementations
void PokerTableWindow::refresh()
{
    // Refresh all UI components
    refreshPot(0); // Will be updated by events
    updateGamePhase(pkt::core::GameState::None);
}

void PokerTableWindow::refreshPot(int amount)
{
    if (m_potLabel) {
        m_potLabel->setText(QString("Pot: $%1").arg(amount));
    }
}

void PokerTableWindow::refreshPlayer(int seat, const pkt::core::player::Player& player)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    
    // All players (including human at seat 0) are now in m_playerComponents
    auto& playerUI = m_playerComponents[seat];
    
    playerUI.nameLabel->setText(QString::fromStdString(player.getName()));
    playerUI.chipsLabel->setText(QString("$%1").arg(player.getCash()));
    
    // Show player area
    playerUI.playerGroup->setVisible(true);
    
    // Set appropriate title based on whether it's human or bot
    if (playerUI.isHuman) {
        playerUI.playerGroup->setTitle(QString("You - %1").arg(QString::fromStdString(player.getName())));
    } else {
        playerUI.playerGroup->setTitle(QString("Bot %1 - %2").arg(seat).arg(QString::fromStdString(player.getName())));
    }
}

void PokerTableWindow::showHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    
    // All players (including human at seat 0) are now in m_playerComponents
    auto& playerUI = m_playerComponents[seat];
    
    if (holeCards.isValid()) {
        playerUI.holeCard1->setPixmap(getCardPixmap(holeCards.card1));
        playerUI.holeCard2->setPixmap(getCardPixmap(holeCards.card2));
    } else {
        playerUI.holeCard1->setPixmap(getCardBackPixmap());
        playerUI.holeCard2->setPixmap(getCardBackPixmap());
    }
}

void PokerTableWindow::showBoardCards(const pkt::core::BoardCards& boardCards)
{
    qDebug() << "showBoardCards called with numCards:" << boardCards.numCards;
    
    // Clear all community cards first
    for (int i = 0; i < 5; ++i) {
        m_communityCards[i]->clear();
        m_communityCards[i]->setVisible(false);
    }
    
    // Show cards based on number of cards dealt
    if (boardCards.numCards >= 3) {
        // Flop
        m_communityCards[0]->setPixmap(getCardPixmap(boardCards.flop1));
        m_communityCards[1]->setPixmap(getCardPixmap(boardCards.flop2));
        m_communityCards[2]->setPixmap(getCardPixmap(boardCards.flop3));
        m_communityCards[0]->setVisible(true);
        m_communityCards[1]->setVisible(true);
        m_communityCards[2]->setVisible(true);
    }
    if (boardCards.numCards >= 4) {
        // Turn
        m_communityCards[3]->setPixmap(getCardPixmap(boardCards.turn));
        m_communityCards[3]->setVisible(true);
    }
    if (boardCards.numCards >= 5) {
        // River
        m_communityCards[4]->setPixmap(getCardPixmap(boardCards.river));
        m_communityCards[4]->setVisible(true);
    }
}

void PokerTableWindow::updateGamePhase(pkt::core::GameState gameState)
{
    QString phaseText;
    switch (gameState) {
        case pkt::core::GameState::None:
            phaseText = "Waiting";
            break;
        case pkt::core::GameState::Preflop:
            phaseText = "Preflop";
            break;
        case pkt::core::GameState::Flop:
            phaseText = "Flop";
            break;
        case pkt::core::GameState::Turn:
            phaseText = "Turn";
            break;
        case pkt::core::GameState::River:
            phaseText = "River";
            break;
        case pkt::core::GameState::PostRiver:
            phaseText = "Showdown";
            break;
    }
    
    // Update the center area round state
    if (m_roundStateLabel) {
        m_roundStateLabel->setText(QString("Phase: %1").arg(phaseText));
    }
}

void PokerTableWindow::updatePlayerStatus(int playerId, const QString& status)
{
    if (playerId == -1) {
        // General game status - no longer used with cleaner UI
        return;
    }
    
    if (playerId < 0 || playerId >= m_maxPlayers) {
        return;
    }
    
    // Auto-highlight active player based on status (visual feedback without clutter)
    if (status.contains("turn", Qt::CaseInsensitive) || 
        status.contains("action", Qt::CaseInsensitive) ||
        status.contains("betting", Qt::CaseInsensitive)) {
        setActivePlayer(playerId);
    } else if (status.contains("fold", Qt::CaseInsensitive) ||
               status.contains("sitting out", Qt::CaseInsensitive)) {
        // Player is no longer active
        if (m_activePlayerId == playerId) {
            clearPlayerHighlights();
        }
    }
}

void PokerTableWindow::showErrorMessage(const QString& message)
{
    // m_statusLabel->setText(QString("Error: %1").arg(message));
    // m_statusLabel->setStyleSheet("color: red;");
    
    // Also show a message box for important errors
    QMessageBox::warning(this, "Poker Game Error", message);
}

void PokerTableWindow::setAvailableActions(const std::vector<pkt::core::ActionType>& actions)
{
    // Disable all actions first
    m_foldButton->setEnabled(false);
    m_callButton->setEnabled(false);
    m_checkButton->setEnabled(false);
    m_betButton->setEnabled(false);
    m_raiseButton->setEnabled(false);
    m_allInButton->setEnabled(false);
    m_bettingGroup->setEnabled(false);
    
    // Enable available actions
    for (const auto& action : actions) {
        switch (action) {
            case pkt::core::ActionType::Fold:
                m_foldButton->setEnabled(true);
                break;
            case pkt::core::ActionType::Call:
                m_callButton->setEnabled(true);
                break;
            case pkt::core::ActionType::Check:
                m_checkButton->setEnabled(true);
                break;
            case pkt::core::ActionType::Bet:
                m_betButton->setEnabled(true);
                m_bettingGroup->setEnabled(true);
                break;
            case pkt::core::ActionType::Raise:
                m_raiseButton->setEnabled(true);
                m_bettingGroup->setEnabled(true);
                break;
            case pkt::core::ActionType::Allin:
                m_allInButton->setEnabled(true);
                break;
            default:
                break;
        }
    }
}

void PokerTableWindow::enablePlayerInput(bool enabled)
{
    m_actionGroup->setEnabled(enabled);
    if (!enabled) {
        m_bettingGroup->setEnabled(false);
        // m_statusLabel->setText("Waiting for other players...");
        // m_statusLabel->setStyleSheet("color: blue;");
    } else {
        // m_statusLabel->setText("Your turn - choose an action");
        // m_statusLabel->setStyleSheet("color: green;");
    }
}

void PokerTableWindow::onHandCompleted()
{
    // Show the Next Hand button when a hand is completed
    m_nextHandButton->setVisible(true);
    
    // Disable action buttons during hand completion
    enablePlayerInput(false);
    
    // Update status to indicate hand is complete
    // m_statusLabel->setText("Hand completed! Click 'Next Hand' to continue.");
    // m_statusLabel->setStyleSheet("color: #2d8659; font-weight: bold;");
}

void PokerTableWindow::resetForNewHand()
{
    qDebug() << "Resetting UI for new hand - clearing community cards";
    
    // Hide the Next Hand button
    m_nextHandButton->setVisible(false);
    
    // Explicitly clear board cards using the same method as game events
    pkt::core::BoardCards emptyBoard; // Default constructor creates preflop state (0 cards)
    showBoardCards(emptyBoard);
    
    // Reset all player hole cards to card backs
    for (auto& player : m_playerComponents) {
        if (player.holeCard1 && player.holeCard2) {
            player.holeCard1->setPixmap(getCardBackPixmap());
            player.holeCard2->setPixmap(getCardBackPixmap());
        }
    }
    
    // Clear player highlights
    clearPlayerHighlights();
    
    // Reset pot display
    refreshPot(0);
    
    // Update round state
    m_roundStateLabel->setText("⏳ Starting new hand...");
    
    // Enable player input for new hand
    enablePlayerInput(true);
}

void PokerTableWindow::positionPlayersInCircle()
{
    // Get the window center and calculate circle radius
    QPoint center(width() / 2, height() / 2);
    int radius = std::min(width(), height()) / 3; // Leave room for the center and margins

    // Position all player areas in a circle
    for (int i = 0; i < m_maxPlayers; ++i) {
        auto& player = m_playerComponents[i];
        if (!player.playerGroup) continue;

        QPoint position = calculateCircularPosition(i, m_maxPlayers, center, radius);
        player.playerGroup->move(position);
        player.playerGroup->show(); // Ensure widget is visible
        player.playerGroup->raise(); // Bring to front
    }
}

QPoint PokerTableWindow::calculateCircularPosition(int playerIndex, int totalPlayers, const QPoint& center, int radius)
{
    // Calculate angle for this player
    // Human player (index 0) should be at bottom (6 o'clock position)
    // Other players arranged clockwise around the circle
    
    double angleStep = 2.0 * M_PI / totalPlayers; // Full circle divided by number of players
    
    // Start angle for bottom position in Qt coordinate system (Y increases downward)
    // Bottom (6 o'clock) = π/2 radians (90 degrees) in Qt coordinates
    double startAngle = M_PI / 2.0; 
    
    // Calculate angle for this specific player
    double angle = startAngle + (playerIndex * angleStep);
    
    // Calculate position on circle using Qt coordinate system
    int x = static_cast<int>(center.x() + radius * std::cos(angle));
    int y = static_cast<int>(center.y() + radius * std::sin(angle));
    
    // Adjust for widget size (center the widget on the calculated point)
    int widgetWidth = 120;  // Fixed size from createPlayerAreas
    int widgetHeight = 160;
    
    x -= widgetWidth / 2;
    y -= widgetHeight / 2;
    
    return QPoint(x, y);
}

void PokerTableWindow::createCenterArea()
{
    // Create center area as invisible container (no background, no border)
    m_centerArea = new QGroupBox(this);
    m_centerArea->setStyleSheet(
        "QGroupBox {"
        "  background: transparent;"
        "  border: none;"
        "  margin: 0px;"
        "  padding: 0px;"
        "}"
    );
    
    auto centerLayout = new QVBoxLayout(m_centerArea);
    centerLayout->setContentsMargins(0, 0, 0, 0); // No margins for clean appearance
    
    // Clean pot information display - text only
    m_potLabel = new QLabel("Pot: $0", this);
    m_potLabel->setAlignment(Qt::AlignCenter);
    m_potLabel->setStyleSheet(
        "QLabel {"
        "  color: #343a40;"
        "  font-size: 22px;"
        "  font-weight: 600;"
        "  margin: 0px;"
        "  background: transparent;"
        "  border: none;"
        "  padding: 8px;"
        "}"
    );
    
    // Clean round state display - text only
    m_roundStateLabel = new QLabel("Waiting for players...", this);
    m_roundStateLabel->setAlignment(Qt::AlignCenter);
    m_roundStateLabel->setStyleSheet(
        "QLabel {"
        "  color: #495057;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  margin: 0px;"
        "  background: transparent;"
        "  border: none;"
        "  padding: 4px;"
        "}"
    );
    
    // Clean community cards layout with elegant styling
    auto communityLayout = new QHBoxLayout();
    communityLayout->setSpacing(8); // Clean spacing between cards
    for (int i = 0; i < 5; ++i) {
        m_communityCards[i] = new QLabel(this);
        m_communityCards[i]->setFixedSize(55, 77); // Smaller card size
        m_communityCards[i]->setScaledContents(true);
        m_communityCards[i]->setStyleSheet(
            "QLabel {"
            "  border: 1px solid #ced4da;"
            "  border-radius: 6px;"
            "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
            "    stop: 0 #ffffff, stop: 1 #f8f9fa);"
            "  margin: 0px;"
            "  padding: 1px;"
            "}"
        );
        m_communityCards[i]->setPixmap(getCardBackPixmap());
        m_communityCards[i]->setVisible(true); // Make visible for demo
        communityLayout->addWidget(m_communityCards[i]);
    }
    
    // Clean center area layout with elegant spacing
    centerLayout->setSpacing(10); // Clean spacing between elements
    centerLayout->addWidget(m_potLabel);
    centerLayout->addSpacing(2); // Small gap
    centerLayout->addWidget(m_roundStateLabel);
    
    // Add spacing before community cards
    centerLayout->addSpacing(12);
    centerLayout->addLayout(communityLayout);
    
    // Set container size for positioning
    m_centerArea->setFixedSize(380, 200); // Clean, proportional size
    
    // Position it properly in the resizeEvent or when the window is shown
    positionCenterArea();
}

void PokerTableWindow::positionCenterArea()
{
    if (m_centerArea) {
        int centerX = (width() - m_centerArea->width()) / 2;
        int centerY = (height() - m_centerArea->height()) / 2;
        m_centerArea->move(centerX, centerY);
    }
}

void PokerTableWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // Reposition all elements when window is resized
    positionPlayersInCircle();
    positionCenterArea();
}

void PokerTableWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // Position elements when window is first shown (now we have proper size)
    positionPlayersInCircle();
    positionCenterArea();
}

// Player state indicator implementations
void PokerTableWindow::setActivePlayer(int playerId)
{
    // Clear previous active player highlighting
    clearPlayerHighlights();
    
    m_activePlayerId = playerId;
    
    // Highlight the active player
    if (playerId >= 0 && playerId < static_cast<int>(m_playerComponents.size())) {
        auto& player = m_playerComponents[playerId];
        if (player.playerGroup) {
            // Apply active player styling with animated border effect
            QString activeStyle;
            if (player.isHuman) {
                activeStyle = 
                    "QGroupBox {"
                    "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                    "    stop: 0 #ffed4e, stop: 0.3 #ffd700, stop: 1 #ffb347);"
                    "  border: 4px solid #ff6b35;"
                    "  border-radius: 18px;"
                    "  margin-top: 12px;"
                    "  color: #8b4513;"
                    "  font-weight: bold;"
                    "  font-size: 14px;"
                    "}"
                    "QGroupBox:title {"
                    "  subcontrol-origin: margin;"
                    "  left: 12px;"
                    "  padding: 0 8px 0 8px;"
                    "  color: #8b0000;"
                    "  font-weight: bold;"
                    "}";
            } else {
                activeStyle = 
                    "QGroupBox {"
                    "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                    "    stop: 0 #ffffff, stop: 0.5 #f0f0f0, stop: 1 #e8e8e8);"
                    "  border: 4px solid #ff6b35;"
                    "  border-radius: 15px;"
                    "  margin-top: 10px;"
                    "  color: #4a4a4a;"
                    "  font-weight: bold;"
                    "}"
                    "QGroupBox:title {"
                    "  subcontrol-origin: margin;"
                    "  left: 10px;"
                    "  padding: 0 5px 0 5px;"
                    "  color: #8b0000;"
                    "  font-weight: bold;"
                    "}";
            }
            player.playerGroup->setStyleSheet(activeStyle);
        }
    }
}

void PokerTableWindow::setDealerPosition(int playerId)
{
    // Hide all dealer buttons first
    for (auto& player : m_playerComponents) {
        if (player.dealerButton) {
            player.dealerButton->setVisible(false);
        }
    }
    
    m_dealerPosition = playerId;
    
    // Show dealer button for the specified player
    if (playerId >= 0 && playerId < static_cast<int>(m_playerComponents.size())) {
        auto& player = m_playerComponents[playerId];
        if (player.dealerButton) {
            player.dealerButton->setVisible(true);
        }
    }
}

void PokerTableWindow::clearPlayerHighlights()
{
    // Reset all players to their default styling
    for (size_t i = 0; i < m_playerComponents.size(); ++i) {
        auto& player = m_playerComponents[i];
        if (player.playerGroup) {
            // Restore original styling
            if (player.isHuman) {
                player.playerGroup->setStyleSheet(
                    "QGroupBox {"
                    "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                    "    stop: 0 #ffd700, stop: 0.3 #ffed4e, stop: 1 #e6c200);"
                    "  border: 3px solid #b8860b;"
                    "  border-radius: 15px;"
                    "  margin-top: 12px;"
                    "  color: #8b4513;"
                    "  font-weight: bold;"
                    "  font-size: 14px;"
                    "}"
                    "QGroupBox:title {"
                    "  subcontrol-origin: margin;"
                    "  left: 12px;"
                    "  padding: 0 8px 0 8px;"
                    "  color: #654321;"
                    "}"
                );
            } else {
                player.playerGroup->setStyleSheet(
                    "QGroupBox {"
                    "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                    "    stop: 0 #f5f5f5, stop: 0.5 #e0e0e0, stop: 1 #d0d0d0);"
                    "  border: 2px solid #a0a0a0;"
                    "  border-radius: 12px;"
                    "  margin-top: 10px;"
                    "  color: #4a4a4a;"
                    "  font-weight: normal;"
                    "}"
                    "QGroupBox:title {"
                    "  subcontrol-origin: margin;"
                    "  left: 10px;"
                    "  padding: 0 5px 0 5px;"
                    "}"
                );
            }
        }
    }
    
    m_activePlayerId = -1;
}

void PokerTableWindow::updatePlayerStateIndicators()
{
    // Refresh the current active player highlighting
    if (m_activePlayerId >= 0) {
        int currentActive = m_activePlayerId;
        clearPlayerHighlights();
        setActivePlayer(currentActive);
    }
    
    // Refresh dealer button
    if (m_dealerPosition >= 0) {
        setDealerPosition(m_dealerPosition);
    }
}

} // namespace pkt::ui::qtwidgets
