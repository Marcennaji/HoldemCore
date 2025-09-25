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
#include <QRegularExpression>
#include <QPoint>
#include <QShowEvent>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>
#include <cmath>
#include <algorithm>

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
    m_cachedHoleCards.resize(m_maxPlayers);  // Cache dealt cards per player
    
    setWindowTitle("Poker Table - HoldemCore");
    setMinimumSize(800, 600);
    
    // Add try-catch around UI setup to catch any crashes
    try {
        setupUi();
        connectSignals();
        
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
    m_cachedHoleCards.clear();
    m_cachedHoleCards.resize(m_maxPlayers);
        
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
        player.playerGroup->setStyleSheet(defaultPlayerGroupStyle());

        auto layout = new QVBoxLayout(player.playerGroup);

        // Hole cards in horizontal layout
        auto cardLayout = new QHBoxLayout();
        player.holeCard1 = new QLabel(this);
        player.holeCard2 = new QLabel(this);

    // Unified card styling and size for all players (scaled with reduced group height)
    player.holeCard1->setFixedSize(50, 70);
    player.holeCard2->setFixedSize(50, 70);
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

        // Prepare opacity effects for fold visualization
        player.card1OpacityEffect = new QGraphicsOpacityEffect(this);
        player.card2OpacityEffect = new QGraphicsOpacityEffect(this);
        player.card1OpacityEffect->setOpacity(1.0);
        player.card2OpacityEffect->setOpacity(1.0);
        player.holeCard1->setGraphicsEffect(player.card1OpacityEffect);
        player.holeCard2->setGraphicsEffect(player.card2OpacityEffect);

        cardLayout->addWidget(player.holeCard1);
        cardLayout->addWidget(player.holeCard2);

        // Prominent current action label
        player.currentActionLabel = new QLabel("", this);
        player.currentActionLabel->setAlignment(Qt::AlignCenter);
    player.currentActionLabel->setStyleSheet(currentActionLabelStyleBase());

        // Winner badge label (hidden by default)
        player.winnerLabel = new QLabel("WINNER", this);
        player.winnerLabel->setAlignment(Qt::AlignCenter);
        player.winnerLabel->setStyleSheet(
            "QLabel {"
            "  color: #155724;"
            "  background: rgba(212, 237, 218, 0.95);"
            "  border: 1px solid #c3e6cb;"
            "  border-radius: 6px;"
            "  font-weight: 800;"
            "  padding: 2px 4px;"
            "}"
        );
        player.winnerLabel->setVisible(false);

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

        layout->addLayout(cardLayout);
    layout->addWidget(player.currentActionLabel);
    layout->addWidget(player.winnerLabel);
        layout->addWidget(player.dealerButton);

    player.playerGroup->setVisible(true);
    player.playerGroup->setFixedSize(120, 136); // 80% of 170
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

    // Unified, sober light style for all action buttons
    QString unifiedBtnStyle =
        "QPushButton {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffffff, stop: 1 #f1f3f5);"
        "  border: 1.5px solid #ced4da;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  color: #343a40;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  min-width: 84px;"
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
        "}";

    m_foldButton->setStyleSheet(unifiedBtnStyle);
    m_callButton->setStyleSheet(unifiedBtnStyle);
    m_checkButton->setStyleSheet(unifiedBtnStyle);
    m_betButton->setStyleSheet(unifiedBtnStyle);
    m_raiseButton->setStyleSheet(unifiedBtnStyle);
    m_allInButton->setStyleSheet(unifiedBtnStyle);

    m_actionLayout->addWidget(m_foldButton);
    m_actionLayout->addWidget(m_callButton);
    m_actionLayout->addWidget(m_checkButton);
    m_actionLayout->addWidget(m_betButton);
    m_actionLayout->addWidget(m_raiseButton);
    m_actionLayout->addWidget(m_allInButton);
}

// Unified style helpers
QString PokerTableWindow::defaultPlayerGroupStyle() const {
    return QString(
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
}

QString PokerTableWindow::activePlayerGroupStyle() const {
    return QString(
        "QGroupBox {"
        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "    stop: 0 #ffffff, stop: 0.5 #f0f0f0, stop: 1 #e8e8e8);"
        "  border: 3px solid #ff6b35;"
        "  border-radius: 12px;"
        "  margin-top: 8px;"
        "  color: #4a4a4a;"
        "  font-weight: bold;"
        "}"
        "QGroupBox:title {"
        "  subcontrol-origin: margin;"
        "  left: 8px;"
        "  padding: 0 4px 0 4px;"
        "  color: #8b0000;"
        "  font-weight: bold;"
        "}"
    );
}

QString PokerTableWindow::currentActionLabelStyleBase() const {
    return QString(
        "QLabel {"
        "  color: #1f3044;"
        "  font-size: 14px;"
        "  font-weight: 700;"
        "  border: 1px solid #cbd5e1;"
        "  border-radius: 6px;"
        "  background: rgba(255,255,255,0.9);"
        "  padding: 4px 6px;"
        "}"
    );
}

QString PokerTableWindow::currentActionLabelStyleFor(const QString& action) const {
    QString base = currentActionLabelStyleBase();
    if (action.compare("FOLD", Qt::CaseInsensitive) == 0) {
        return base + " QLabel { color: #b00020; border-color: #f5c2c7; background: #fde7ea; }";
    }
    if (action.compare("CALL", Qt::CaseInsensitive) == 0 || action.compare("CHECK", Qt::CaseInsensitive) == 0) {
        return base + " QLabel { color: #1b5e20; border-color: #c8e6c9; background: #e8f5e9; }";
    }
    if (action.compare("BET", Qt::CaseInsensitive) == 0 || action.compare("RAISE", Qt::CaseInsensitive) == 0 || action.contains("ALL", Qt::CaseInsensitive)) {
        return base + " QLabel { color: #0d47a1; border-color: #bbdefb; background: #e3f2fd; }";
    }
    return base;
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

    m_betAmountLabel->setStyleSheet("color: #495057; font-size: 14px;");

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

    // Enhanced betting controls styling (sober, light)
    QString sliderStyle = 
        "QSlider::groove:horizontal {"
        "  border: 1px solid #ced4da;"
        "  height: 10px;"
        "  background: #f1f3f5;"
        "  border-radius: 5px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #adb5bd;"
        "  border: 1px solid #868e96;"
        "  width: 18px;"
        "  margin: -6px 0;"
        "  border-radius: 9px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: #868e96;"
        "}";

    QString spinBoxStyle = 
        "QSpinBox {"
        "  background-color: #ffffff;"
        "  border: 1px solid #ced4da;"
        "  border-radius: 6px;"
        "  padding: 6px;"
        "  color: #343a40;"
        "  font-size: 14px;"
        "  min-width: 100px;"
        "}"
        "QSpinBox:focus {"
        "  border: 1px solid #adb5bd;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  background: #f1f3f5;"
        "  border: 1px solid #ced4da;"
        "  width: 18px;"
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        "  background: #e9ecef;"
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
    // Also reset bet controls after any action click
    connect(m_foldButton, &QPushButton::clicked, this, [this]() { resetBetControls(); });
    connect(m_callButton, &QPushButton::clicked, this, [this]() { resetBetControls(); });
    connect(m_checkButton, &QPushButton::clicked, this, [this]() { resetBetControls(); });
    connect(m_allInButton, &QPushButton::clicked, this, [this]() { resetBetControls(); });
    
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
    // Reset the betting controls after action
    resetBetControls();
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

    // Show player area
    playerUI.playerGroup->setVisible(true);
    
    // Title contains name and cash
    QString prefix = playerUI.isHuman ? "You" : QString("Bot %1").arg(seat);
    playerUI.playerGroup->setTitle(QString("%1 — %2 ($%3)")
        .arg(prefix)
        .arg(QString::fromStdString(player.getName()))
        .arg(player.getCash()));
}

void PokerTableWindow::showHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    
    // All players (including human at seat 0) are now in m_playerComponents
    auto& playerUI = m_playerComponents[seat];
    
    if (holeCards.isValid() && !playerUI.isFolded) {
        playerUI.holeCard1->setPixmap(getCardPixmap(holeCards.card1));
        playerUI.holeCard2->setPixmap(getCardPixmap(holeCards.card2));
        // Cache for potential showdown reveal
        cacheHoleCards(seat, holeCards);
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
        // We definitely saw the river; use this to infer showdown in case events arrive quickly
        m_sawRiver = true;
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
            setReachedShowdown(true);
            break;
    }
    
    // Update the center area round state
    if (m_roundStateLabel) {
        m_roundStateLabel->setText(QString("Phase: %1").arg(phaseText));
    }
}

void PokerTableWindow::clearActionLabelsForNewRound()
{
    for (auto& p : m_playerComponents) {
        if (p.currentActionLabel) {
            p.currentActionLabel->clear();
        }
    }
}

void PokerTableWindow::clearPlayerActionLabel(int playerId)
{
    if (playerId < 0 || playerId >= m_maxPlayers) return;
    auto& ui = m_playerComponents[playerId];
    if (ui.currentActionLabel) {
        ui.currentActionLabel->clear();
    }
}

void PokerTableWindow::updatePlayerStatus(int playerId, const QString& status)
{
    // We no longer parse player-specific statuses by string; controller calls structured APIs.
    // Keep this as a no-op for playerId >= 0 to avoid false positives from free-form text.
    if (playerId == -1) {
        return; // general game status ignored in this UI variant
    }
    return;
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

// Structured UI updates (no string parsing)
void PokerTableWindow::showPlayerAction(int playerId, pkt::core::ActionType action, int amount)
{
    if (playerId < 0 || playerId >= m_maxPlayers) return;
    auto& ui = m_playerComponents[playerId];

    QString text;
    switch (action) {
        case pkt::core::ActionType::Fold:
            text = "FOLD";
            applyFoldVisual(playerId, true);
            break;
        case pkt::core::ActionType::Check:
            text = "CHECK";
            break;
        case pkt::core::ActionType::Call:
            text = "CALL";
            break;
        case pkt::core::ActionType::Bet:
            text = amount > 0 ? QString("BET %1").arg(amount) : "BET";
            break;
        case pkt::core::ActionType::Raise:
            text = amount > 0 ? QString("RAISE %1").arg(amount) : "RAISE";
            break;
        case pkt::core::ActionType::Allin:
            text = "ALL-IN";
            break;
        case pkt::core::ActionType::PostSmallBlind:
            text = amount > 0 ? QString("SB %1").arg(amount) : "SB";
            break;
        case pkt::core::ActionType::PostBigBlind:
            text = amount > 0 ? QString("BB %1").arg(amount) : "BB";
            break;
        default:
            text = QString::fromUtf8(actionTypeToString(action)).toUpper();
            break;
    }

    if (ui.currentActionLabel) {
        ui.currentActionLabel->setText(text);
        ui.currentActionLabel->setStyleSheet(currentActionLabelStyleFor(text));
    }
}

void PokerTableWindow::showPlayerTurn(int playerId)
{
    if (playerId < 0 || playerId >= m_maxPlayers) return;
    auto& ui = m_playerComponents[playerId];
    // Do NOT overwrite the player's last action during the betting round.
    // We only indicate turn via highlighting to keep last action visible.
    setActivePlayer(playerId);
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
    for (int i = 0; i < static_cast<int>(m_playerComponents.size()); ++i) {
        auto& player = m_playerComponents[i];
        if (player.holeCard1 && player.holeCard2) {
            player.holeCard1->setPixmap(getCardBackPixmap());
            player.holeCard2->setPixmap(getCardBackPixmap());
        }
    // Preserve last action label until the next betting round starts.
    // clearActionLabelsForNewRound() will clear them on GameState change.
        if (player.winnerLabel) {
            player.winnerLabel->clear();
            player.winnerLabel->setVisible(false);
        }
        applyFoldVisual(i, false);
    }

    // Reset showdown flags and caches
    m_reachedShowdown = false;
    m_sawRiver = false;
    if (!m_cachedHoleCards.empty()) {
        for (auto& hc : m_cachedHoleCards) {
            hc.reset();
        }
    }
    
    // Clear player highlights
    clearPlayerHighlights();
    
    // Hide dealer indicator until blinds are posted for the new hand
    for (auto& player : m_playerComponents) {
        if (player.dealerButton) {
            player.dealerButton->setVisible(false);
        }
    }
    m_dealerPosition = -1;

    // Reset pot display
    refreshPot(0);
    
    // Update round state
    m_roundStateLabel->setText("⏳ Starting new hand...");
    
    // Enable player input for new hand
    enablePlayerInput(true);
}

void PokerTableWindow::showWinners(const std::list<unsigned>& winnerIds, int totalPot)
{
    // Show a winner badge over each winner's area. If multiple, display all.
    const bool isSplit = winnerIds.size() > 1;
    for (unsigned id : winnerIds) {
        if (id < m_playerComponents.size()) {
            auto& player = m_playerComponents[id];
            if (player.winnerLabel) {
                if (isSplit) {
                    // Avoid implying the full pot went to each; indicate split
                    player.winnerLabel->setText("WINNER (split)");
                } else {
                    player.winnerLabel->setText(QString("WINNER +$%1").arg(totalPot));
                }
                player.winnerLabel->setVisible(true);
            }
            // If we reached showdown (not everyone folded), reveal the winner's hole cards
            if ((m_reachedShowdown || m_sawRiver || isSplit) && id < m_cachedHoleCards.size()) {
                const auto& hc = m_cachedHoleCards[id];
                if (hc.isValid()) {
                    showHoleCards(static_cast<int>(id), hc);
                }
            }
        }
    }

}

void PokerTableWindow::revealShowdownOrder(const std::vector<unsigned>& revealOrder)
{
    // Reveal strictly according to engine-provided order. Use cached hole cards.
    for (unsigned id : revealOrder) {
        if (id < m_playerComponents.size() && id < m_cachedHoleCards.size()) {
            auto& ui = m_playerComponents[id];
            const auto& hc = m_cachedHoleCards[id];
            if (!ui.isFolded && hc.isValid()) {
                showHoleCards(static_cast<int>(id), hc);
            }
        }
        // slight UI pacing to make reveal readable
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }
}

void PokerTableWindow::applyFoldVisual(int seat, bool folded)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    auto& p = m_playerComponents[seat];
    p.isFolded = folded;
    if (p.card1OpacityEffect && p.card2OpacityEffect) {
        p.card1OpacityEffect->setOpacity(folded ? 0.35 : 1.0);
        p.card2OpacityEffect->setOpacity(folded ? 0.35 : 1.0);
    }
    if (folded) {
        if (p.holeCard1) p.holeCard1->setPixmap(getCardBackPixmap());
        if (p.holeCard2) p.holeCard2->setPixmap(getCardBackPixmap());
    }
}

void PokerTableWindow::positionPlayersInCircle()
{
    // Compute using full height for a larger circle, and then clamp center Y so bottom stays above controls
    const int bottomReserve = reservedBottomHeight();
    const int fullHeight = height();
    QPoint center(width() / 2, fullHeight / 2);

    // Player widget half-height and half-width
    const int widgetHalfH = 136 / 2;
    const int widgetHalfW = 120 / 2;

    // Clearance from the center area (community cards) in both axes
    const int centerHalfH = m_centerArea ? (m_centerArea->height() / 2) : 90;
    const int centerHalfW = m_centerArea ? (m_centerArea->width() / 2) : 180;
    const int minVerticalRadius = centerHalfH + widgetHalfH + 32;
    const int minHorizontalRadius = centerHalfW + widgetHalfW + 32;

    // Edge margins
    const int edgeMargin = 16;

    // Maximum radius constrained by available width/height (full height for larger circle)
    const int maxRadiusByHeight = std::max(0, fullHeight / 2 - widgetHalfH - edgeMargin);
    const int maxRadiusByWidth = std::max(0, width() / 2 - widgetHalfW - edgeMargin);
    int radius = std::min({ maxRadiusByHeight, maxRadiusByWidth });

    // Ensure minimal clearance from center area
    const int desiredMin = std::max(minVerticalRadius, minHorizontalRadius);
    radius = std::max(radius, desiredMin);

    // Hard cap to avoid off-screen placement
    const int hardCap = std::min(width(), fullHeight) / 2 - std::max(widgetHalfH, widgetHalfW) - edgeMargin;
    radius = std::min(radius, std::max(0, hardCap));

    // Reduce the radius to 90% of current while respecting minimum clearance
    const double radiusScale = 0.90;
    radius = static_cast<int>(std::round(radius * radiusScale));
    if (radius < desiredMin) radius = desiredMin;

    // Clamp center Y so bottom-most player stays above the reserved controls area (with extra padding)
    const int bottomExtraPadding = 32; // Additional breathing room above action controls
    const int allowedBottom = fullHeight - bottomReserve - edgeMargin - bottomExtraPadding; // y for bottom edge of player widget
    const int maxCy = allowedBottom - radius - widgetHalfH;             // cy + radius + widgetHalfH <= allowedBottom
    const int minCy = edgeMargin + radius + widgetHalfH;                // top-most player not clipped
    int cy = std::clamp(center.y(), minCy, std::max(minCy, maxCy));
    center.setY(cy);

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
    int widgetHeight = 136; // Match created height (80% of previous)
    
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
    communityLayout->setSpacing(4); // Tighter spacing between cards to reduce overall width
    for (int i = 0; i < 5; ++i) {
        m_communityCards[i] = new QLabel(this);
        m_communityCards[i]->setFixedSize(50, 70); // Slightly smaller cards to save space
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
    m_centerArea->setFixedSize(360, 180); // Smaller footprint to avoid overlap with side players
    
    // Position it properly in the resizeEvent or when the window is shown
    positionCenterArea();
}

void PokerTableWindow::positionCenterArea()
{
    if (m_centerArea) {
        const int bottomReserve = reservedBottomHeight();
        const int effectiveHeight = std::max(0, height() - bottomReserve);
        int centerX = (width() - m_centerArea->width()) / 2;
        int centerY = (effectiveHeight - m_centerArea->height()) / 2;
        m_centerArea->move(centerX, centerY);
        // Ensure the center area (and community cards) render above players if close
        m_centerArea->raise();
    }
}

void PokerTableWindow::cacheHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    if (static_cast<size_t>(seat) >= m_cachedHoleCards.size()) {
        m_cachedHoleCards.resize(m_maxPlayers);
    }
    m_cachedHoleCards[static_cast<size_t>(seat)] = holeCards;
}

void PokerTableWindow::setReachedShowdown(bool reached)
{
    m_reachedShowdown = reached;
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

int PokerTableWindow::reservedBottomHeight() const
{
    // Reserve enough vertical space to fit: action group, betting group, and next hand button rows.
    // If widgets exist, use their size hints; otherwise fall back to conservative defaults.
    int actionH = m_actionGroup ? m_actionGroup->sizeHint().height() : 70;
    int bettingH = m_bettingGroup ? m_bettingGroup->sizeHint().height() : 60;
    int nextHandH = m_nextHandButton ? (m_nextHandButton->sizeHint().height() + 12) : 40;
    // Include some padding and layout spacing
    int padding = 24; // extra gap between human cards and controls
    return actionH + bettingH + nextHandH + padding;
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
            // Apply unified active player styling
            player.playerGroup->setStyleSheet(activePlayerGroupStyle());
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

void PokerTableWindow::setDealerFromSmallBlind(int smallBlindId)
{
    if (smallBlindId < 0 || smallBlindId >= m_maxPlayers) return;
    // Heads-up: Button is also Small Blind
    int dealerId = smallBlindId;
    if (m_maxPlayers > 2) {
        // 3+ players: Dealer is immediately to the right of Small Blind (previous seat modulo N)
        dealerId = smallBlindId - 1;
        if (dealerId < 0) dealerId = m_maxPlayers - 1;
    }
    setDealerPosition(dealerId);
}

void PokerTableWindow::resetBetControls()
{
    if (m_betSlider) m_betSlider->setValue(0);
    if (m_betSpinBox) m_betSpinBox->setValue(0);
    if (m_betAmountLabel) m_betAmountLabel->setText("Amount: $0");
}

void PokerTableWindow::clearPlayerHighlights()
{
    // Reset all players to their default styling
    for (size_t i = 0; i < m_playerComponents.size(); ++i) {
        auto& player = m_playerComponents[i];
        if (player.playerGroup) {
            // Restore unified default styling
            player.playerGroup->setStyleSheet(defaultPlayerGroupStyle());
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
