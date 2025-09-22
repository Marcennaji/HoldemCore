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

namespace pkt::ui::qtwidgets
{

PokerTableWindow::PokerTableWindow(pkt::core::Session* session, QWidget* parent) 
    : QWidget(parent), m_session(session), m_maxPlayers(6) // Default to 6 players
{
    // Add null check for session
    if (!session) {
        // This could cause issues later
    }
    
    // Initialize bot player components vector (excluding human player)
    m_playerComponents.resize(m_maxPlayers - 1); // -1 for human player
    
    setWindowTitle("Poker Table - HoldemCore");
    setMinimumSize(800, 600);
    
    // Add try-catch around UI setup to catch any crashes
    try {
        setupUi();
        connectSignals();
    } catch (...) {
        // Catch any exception during UI setup
    }
}

void PokerTableWindow::initializeWithGameData(const pkt::core::GameData& gameData)
{
    // Update max players and resize components if needed
    if (gameData.maxNumberOfPlayers != m_maxPlayers) {
        m_maxPlayers = gameData.maxNumberOfPlayers;
        
        // Clear existing bot player components
        for (auto& component : m_playerComponents) {
            if (component.playerGroup) {
                component.playerGroup->setParent(nullptr);
                delete component.playerGroup;
            }
        }
        
        // Resize and recreate bot player components (excluding human player)
        m_playerComponents.clear();
        m_playerComponents.resize(m_maxPlayers - 1); // -1 for human player
        
        // Recreate UI with new player count
        createPlayerAreas();
        
        // Update layout for bot players only
        auto playerLayout = qobject_cast<QHBoxLayout*>(m_mainLayout->itemAt(1)->layout());
        if (playerLayout) {
            for (int i = 0; i < m_maxPlayers - 1; ++i) { // -1 for human player
                playerLayout->addWidget(m_playerComponents[i].playerGroup);
            }
        }
        
        // Human player area is created separately and doesn't need dynamic resizing
    }
}

void PokerTableWindow::setupUi()
{
    // Create the main layout
    m_mainLayout = new QVBoxLayout(this);

    // Create all UI components properly
    createGameInfoArea();
    createPlayerAreas();
    createHumanPlayerArea();
    createBoardArea();
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

    // Add components to main layout
    m_mainLayout->addWidget(m_gameInfoGroup);

    // Add bot player areas in a horizontal layout (excluding human player)
    auto playerLayout = new QHBoxLayout();
    for (int i = 0; i < m_maxPlayers - 1; ++i) { // -1 because human player is separate
        playerLayout->addWidget(m_playerComponents[i].playerGroup);
    }
    m_mainLayout->addLayout(playerLayout);

    m_mainLayout->addWidget(m_boardGroup);
    
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
    
    // Add human player area at the bottom
    m_mainLayout->addWidget(m_humanPlayerGroup);

    // Set layout and style
    this->setLayout(m_mainLayout);

    // Elegant light background
    this->setStyleSheet("background-color: #f7f7f7;");

    this->resize(1200, 800);

    this->show();
    this->raise();
    this->activateWindow();
}

void PokerTableWindow::createGameInfoArea()
{
    m_gameInfoGroup = new QGroupBox("Game Information", this);
    m_gameInfoGroup->setStyleSheet(
        "QGroupBox {"
        "  background-color: #ffffff;"
        "  border: 1.5px solid #b0b0b0;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "  font-weight: bold;"
        "  color: #333;"
        "}"
        "QGroupBox:title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 3px 0 3px;"
        "}"
    );
    m_gameInfoLayout = new QHBoxLayout(m_gameInfoGroup);

    m_potLabel = new QLabel("Pot: $0", this);
    m_gamePhaseLabel = new QLabel("Phase: Waiting", this);
    m_statusLabel = new QLabel("Status: Ready", this);

    m_potLabel->setStyleSheet("color: #2d8659; font-size: 16px;");
    m_gamePhaseLabel->setStyleSheet("color: #5a5a5a; font-size: 16px;");
    m_statusLabel->setStyleSheet("color: #888; font-size: 16px;");

    m_gameInfoLayout->addWidget(m_potLabel);
    m_gameInfoLayout->addWidget(m_gamePhaseLabel);
    m_gameInfoLayout->addWidget(m_statusLabel);
    m_gameInfoLayout->addStretch();
}

void PokerTableWindow::createPlayerAreas()
{
    // Create UI components for bot players only (excluding human player at ID 0)
    for (int i = 0; i < m_maxPlayers - 1; ++i) { // -1 because human player is separate
        auto& player = m_playerComponents[i];

        player.playerGroup = new QGroupBox(QString("Bot %1").arg(i + 1), this);
        player.playerGroup->setStyleSheet(
            "QGroupBox {"
            "  background-color: #f0f4f8;"
            "  border: 1px solid #d0d7de;"
            "  border-radius: 8px;"
            "  margin-top: 8px;"
            "  color: #444;"
            "  font-weight: normal;"
            "}"
            "QGroupBox:title {"
            "  subcontrol-origin: margin;"
            "  left: 8px;"
            "  padding: 0 3px 0 3px;"
            "}"
        );
        auto layout = new QVBoxLayout(player.playerGroup);

        player.nameLabel = new QLabel("Empty", this);
        player.chipsLabel = new QLabel("$0", this);
        player.statusLabel = new QLabel("Sitting Out", this);

        player.nameLabel->setStyleSheet("color: #2d4059; font-size: 14px;");
        player.chipsLabel->setStyleSheet("color: #3a7ca5; font-size: 13px;");
        player.statusLabel->setStyleSheet("color: #888; font-size: 12px;");

        // Hole cards in horizontal layout
        auto cardLayout = new QHBoxLayout();
        player.holeCard1 = new QLabel(this);
        player.holeCard2 = new QLabel(this);

        player.holeCard1->setFixedSize(50, 70);
        player.holeCard2->setFixedSize(50, 70);
        player.holeCard1->setScaledContents(true);
        player.holeCard2->setScaledContents(true);

        player.holeCard1->setPixmap(getCardBackPixmap());
        player.holeCard2->setPixmap(getCardBackPixmap());

        cardLayout->addWidget(player.holeCard1);
        cardLayout->addWidget(player.holeCard2);

        layout->addWidget(player.nameLabel);
        layout->addWidget(player.chipsLabel);
        layout->addWidget(player.statusLabel);
        layout->addLayout(cardLayout);

        player.playerGroup->setVisible(true);
    }
}

void PokerTableWindow::createHumanPlayerArea()
{
    // Create dedicated area for human player at the bottom
    m_humanPlayerGroup = new QGroupBox("Your Cards", this);
    m_humanPlayerGroup->setStyleSheet(
        "QGroupBox {"
        "  background-color: #e8f4f8;"
        "  border: 2px solid #2c5282;"
        "  border-radius: 12px;"
        "  margin-top: 10px;"
        "  color: #2c5282;"
        "  font-weight: bold;"
        "  font-size: 16px;"
        "}"
        "QGroupBox:title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px 0 5px;"
        "}"
    );
    
    m_humanPlayerLayout = new QHBoxLayout(m_humanPlayerGroup);
    
    // Create a container for player info (left side)
    auto infoLayout = new QVBoxLayout();
    
    m_humanNameLabel = new QLabel("You", this);
    m_humanChipsLabel = new QLabel("$0", this);
    m_humanStatusLabel = new QLabel("Ready", this);
    
    m_humanNameLabel->setStyleSheet("color: #2c5282; font-size: 18px; font-weight: bold;");
    m_humanChipsLabel->setStyleSheet("color: #2c5282; font-size: 16px; font-weight: bold;");
    m_humanStatusLabel->setStyleSheet("color: #2c5282; font-size: 14px;");
    
    infoLayout->addWidget(m_humanNameLabel);
    infoLayout->addWidget(m_humanChipsLabel);
    infoLayout->addWidget(m_humanStatusLabel);
    
    // Create cards layout (right side) - larger cards for human player
    auto cardLayout = new QHBoxLayout();
    m_humanHoleCard1 = new QLabel(this);
    m_humanHoleCard2 = new QLabel(this);
    
    m_humanHoleCard1->setFixedSize(80, 112); // Larger cards for human player
    m_humanHoleCard2->setFixedSize(80, 112);
    m_humanHoleCard1->setScaledContents(true);
    m_humanHoleCard2->setScaledContents(true);
    
    m_humanHoleCard1->setPixmap(getCardBackPixmap());
    m_humanHoleCard2->setPixmap(getCardBackPixmap());
    
    cardLayout->addWidget(m_humanHoleCard1);
    cardLayout->addWidget(m_humanHoleCard2);
    
    // Add both layouts to the main horizontal layout
    m_humanPlayerLayout->addLayout(infoLayout);
    m_humanPlayerLayout->addStretch(); // Add space between info and cards
    m_humanPlayerLayout->addLayout(cardLayout);
    
    m_humanPlayerGroup->setVisible(true);
}

void PokerTableWindow::createBoardArea()
{
    m_boardGroup = new QGroupBox("Community Cards", this);
    m_boardGroup->setStyleSheet(
        "QGroupBox {"
        "  background-color: #e9ecef;"
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
    m_boardLayout = new QHBoxLayout(m_boardGroup);

    for (int i = 0; i < 5; ++i) {
        m_boardCardLabels[i] = new QLabel(this);
        m_boardCardLabels[i]->setFixedSize(60, 84);
        m_boardCardLabels[i]->setScaledContents(true);
        m_boardCardLabels[i]->setStyleSheet(
            "border: 1px solid #b0b0b0; background-color: #fff; border-radius: 4px;"
        );
        m_boardLayout->addWidget(m_boardCardLabels[i]);
    }

    m_boardLayout->addStretch();
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

    // Elegant button styles - optimized for side-by-side layout
    QString btnStyle =
        "QPushButton {"
        "  background-color: #e3eafc;"
        "  border: 1px solid #b0b0b0;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"  // Slightly more compact padding
        "  color: #2d4059;"
        "  font-size: 14px;"    // Slightly smaller font
        "  font-weight: 500;"
        "  min-width: 60px;"    // Ensure minimum width
        "}"
        "QPushButton:disabled {"
        "  background-color: #f0f0f0;"
        "  color: #aaa;"
        "  border: 1px solid #e0e0e0;"
        "}";

    m_foldButton->setStyleSheet(btnStyle);
    m_callButton->setStyleSheet(btnStyle);
    m_checkButton->setStyleSheet(btnStyle);
    m_betButton->setStyleSheet(btnStyle);
    m_raiseButton->setStyleSheet(btnStyle);
    m_allInButton->setStyleSheet(btnStyle);

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
    m_statusLabel->setText("Starting next hand...");
    m_statusLabel->setStyleSheet("color: #2d8659;");
    
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
    m_potLabel->setText(QString("Pot: $%1").arg(amount));
}

void PokerTableWindow::refreshPlayer(int seat, const pkt::core::player::Player& player)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    
    if (seat == 0) {
        // Handle human player (seat 0) - update human player area
        m_humanNameLabel->setText(QString::fromStdString(player.getName()));
        m_humanChipsLabel->setText(QString("$%1").arg(player.getCash()));
        
        // Update status based on player state - you can expand this based on your player state logic
        m_humanStatusLabel->setText("Playing");
        
        m_humanPlayerGroup->setVisible(true);
    } else {
        // Handle bot players (seat 1+) - map to bot player components
        int botIndex = seat - 1; // Convert seat to bot index (seat 1 -> bot index 0, etc.)
        if (botIndex < 0 || botIndex >= static_cast<int>(m_playerComponents.size())) return;
        
        auto& playerUI = m_playerComponents[botIndex];
        
        playerUI.nameLabel->setText(QString::fromStdString(player.getName()));
        playerUI.chipsLabel->setText(QString("$%1").arg(player.getCash()));
        
        // Show player area
        playerUI.playerGroup->setVisible(true);
        playerUI.playerGroup->setTitle(QString("Bot %1 - %2").arg(botIndex + 1).arg(QString::fromStdString(player.getName())));
    }
}

void PokerTableWindow::showHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers) return;
    
    if (seat == 0) {
        // Handle human player (seat 0) - update human player cards
        if (holeCards.isValid()) {
            m_humanHoleCard1->setPixmap(getCardPixmap(holeCards.card1));
            m_humanHoleCard2->setPixmap(getCardPixmap(holeCards.card2));
        } else {
            m_humanHoleCard1->setPixmap(getCardBackPixmap());
            m_humanHoleCard2->setPixmap(getCardBackPixmap());
        }
    } else {
        // Handle bot players (seat 1+) - map to bot player components
        int botIndex = seat - 1; // Convert seat to bot index (seat 1 -> bot index 0, etc.)
        if (botIndex < 0 || botIndex >= static_cast<int>(m_playerComponents.size())) return;
        
        auto& playerUI = m_playerComponents[botIndex];
        
        if (holeCards.isValid()) {
            playerUI.holeCard1->setPixmap(getCardPixmap(holeCards.card1));
            playerUI.holeCard2->setPixmap(getCardPixmap(holeCards.card2));
        } else {
            playerUI.holeCard1->setPixmap(getCardBackPixmap());
            playerUI.holeCard2->setPixmap(getCardBackPixmap());
        }
    }
}

void PokerTableWindow::showBoardCards(const pkt::core::BoardCards& boardCards)
{
    // Clear all board cards first
    for (int i = 0; i < 5; ++i) {
        m_boardCardLabels[i]->clear();
        m_boardCardLabels[i]->setStyleSheet("border: 1px solid gray; background-color: lightgray;");
    }
    
    // Show cards based on number of cards dealt
    if (boardCards.numCards >= 3) {
        // Flop
        m_boardCardLabels[0]->setPixmap(getCardPixmap(boardCards.flop1));
        m_boardCardLabels[1]->setPixmap(getCardPixmap(boardCards.flop2));
        m_boardCardLabels[2]->setPixmap(getCardPixmap(boardCards.flop3));
    }
    if (boardCards.numCards >= 4) {
        // Turn
        m_boardCardLabels[3]->setPixmap(getCardPixmap(boardCards.turn));
    }
    if (boardCards.numCards >= 5) {
        // River
        m_boardCardLabels[4]->setPixmap(getCardPixmap(boardCards.river));
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
    m_gamePhaseLabel->setText(QString("Phase: %1").arg(phaseText));
}

void PokerTableWindow::updatePlayerStatus(int playerId, const QString& status)
{
    if (playerId == -1) {
        // General game status - update the main status label
        m_statusLabel->setText(status);
        m_statusLabel->setStyleSheet("color: green;");
        return;
    }
    
    if (playerId < 0 || playerId >= m_maxPlayers) {
        return;
    }
    
    if (playerId == 0) {
        // Handle human player (ID 0) - update human player status
        m_humanStatusLabel->setText(status);
    } else {
        // Handle bot players (ID 1+) - map to bot player components
        int botIndex = playerId - 1; // Convert player ID to bot index
        if (botIndex < 0 || botIndex >= static_cast<int>(m_playerComponents.size())) return;
        
        m_playerComponents[botIndex].statusLabel->setText(status);
    }
}

void PokerTableWindow::showErrorMessage(const QString& message)
{
    m_statusLabel->setText(QString("Error: %1").arg(message));
    m_statusLabel->setStyleSheet("color: red;");
    
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
        m_statusLabel->setText("Waiting for other players...");
        m_statusLabel->setStyleSheet("color: blue;");
    } else {
        m_statusLabel->setText("Your turn - choose an action");
        m_statusLabel->setStyleSheet("color: green;");
    }
}

void PokerTableWindow::onHandCompleted()
{
    // Show the Next Hand button when a hand is completed
    m_nextHandButton->setVisible(true);
    
    // Disable action buttons during hand completion
    enablePlayerInput(false);
    
    // Update status to indicate hand is complete
    m_statusLabel->setText("Hand completed! Click 'Next Hand' to continue.");
    m_statusLabel->setStyleSheet("color: #2d8659; font-weight: bold;");
}
} // namespace pkt::ui::qtwidgets
