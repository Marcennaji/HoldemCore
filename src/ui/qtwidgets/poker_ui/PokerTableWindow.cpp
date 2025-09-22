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
    : QWidget(parent), m_session(session)
{
    setupUi();
    connectSignals();
}

void PokerTableWindow::setupUi()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    
    createGameInfoArea();
    createPlayerAreas();
    createBoardArea();
    createActionButtons();
    createBettingControls();
    
    // Add everything to main layout
    m_mainLayout->addWidget(m_gameInfoGroup);
    
    // Create table layout for players and board
    m_tableLayout = new QGridLayout();
    
    // Add player positions around the table (9-max layout)
    // Positions: 0-4 on top row, 5-8 on bottom row
    for (int i = 0; i < 5; ++i) {
        m_tableLayout->addWidget(m_playerComponents[i].playerGroup, 0, i);
    }
    for (int i = 5; i < 9; ++i) {
        m_tableLayout->addWidget(m_playerComponents[i].playerGroup, 2, i - 5);
    }
    
    // Board in the center
    m_tableLayout->addWidget(m_boardGroup, 1, 1, 1, 3);
    
    m_mainLayout->addLayout(m_tableLayout);
    m_mainLayout->addWidget(m_actionGroup);
    m_mainLayout->addWidget(m_bettingGroup);
}

void PokerTableWindow::createGameInfoArea()
{
    m_gameInfoGroup = new QGroupBox("Game Information", this);
    m_gameInfoLayout = new QHBoxLayout(m_gameInfoGroup);
    
    m_potLabel = new QLabel("Pot: $0", this);
    m_gamePhaseLabel = new QLabel("Phase: Waiting", this);
    m_statusLabel = new QLabel("Status: Ready", this);
    
    m_gameInfoLayout->addWidget(m_potLabel);
    m_gameInfoLayout->addWidget(m_gamePhaseLabel);
    m_gameInfoLayout->addWidget(m_statusLabel);
    m_gameInfoLayout->addStretch();
}

void PokerTableWindow::createPlayerAreas()
{
    for (int i = 0; i < 9; ++i) {
        auto& player = m_playerComponents[i];
        
        player.playerGroup = new QGroupBox(QString("Seat %1").arg(i + 1), this);
        auto layout = new QVBoxLayout(player.playerGroup);
        
        player.nameLabel = new QLabel("Empty", this);
        player.chipsLabel = new QLabel("$0", this);
        player.statusLabel = new QLabel("Sitting Out", this);
        
        // Hole cards in horizontal layout
        auto cardLayout = new QHBoxLayout();
        player.holeCard1 = new QLabel(this);
        player.holeCard2 = new QLabel(this);
        
        // Set fixed size for cards
        player.holeCard1->setFixedSize(50, 70);
        player.holeCard2->setFixedSize(50, 70);
        player.holeCard1->setScaledContents(true);
        player.holeCard2->setScaledContents(true);
        
        // Show card backs initially
        player.holeCard1->setPixmap(getCardBackPixmap());
        player.holeCard2->setPixmap(getCardBackPixmap());
        
        cardLayout->addWidget(player.holeCard1);
        cardLayout->addWidget(player.holeCard2);
        
        layout->addWidget(player.nameLabel);
        layout->addWidget(player.chipsLabel);
        layout->addWidget(player.statusLabel);
        layout->addLayout(cardLayout);
        
        // Initially hide empty seats
        player.playerGroup->setVisible(false);
    }
}

void PokerTableWindow::createBoardArea()
{
    m_boardGroup = new QGroupBox("Community Cards", this);
    m_boardLayout = new QHBoxLayout(m_boardGroup);
    
    for (int i = 0; i < 5; ++i) {
        m_boardCardLabels[i] = new QLabel(this);
        m_boardCardLabels[i]->setFixedSize(60, 84);
        m_boardCardLabels[i]->setScaledContents(true);
        m_boardCardLabels[i]->setStyleSheet("border: 1px solid gray; background-color: lightgray;");
        m_boardLayout->addWidget(m_boardCardLabels[i]);
    }
    
    m_boardLayout->addStretch();
}

void PokerTableWindow::createActionButtons()
{
    m_actionGroup = new QGroupBox("Actions", this);
    m_actionLayout = new QHBoxLayout(m_actionGroup);
    
    m_foldButton = new QPushButton("Fold", this);
    m_callButton = new QPushButton("Call", this);
    m_checkButton = new QPushButton("Check", this);
    m_betButton = new QPushButton("Bet", this);
    m_raiseButton = new QPushButton("Raise", this);
    m_allInButton = new QPushButton("All-In", this);
    
    m_actionLayout->addWidget(m_foldButton);
    m_actionLayout->addWidget(m_callButton);
    m_actionLayout->addWidget(m_checkButton);
    m_actionLayout->addWidget(m_betButton);
    m_actionLayout->addWidget(m_raiseButton);
    m_actionLayout->addWidget(m_allInButton);
    
    // Initially disable all buttons
    enablePlayerInput(false);
}

void PokerTableWindow::createBettingControls()
{
    m_bettingGroup = new QGroupBox("Betting Amount", this);
    m_bettingLayout = new QHBoxLayout(m_bettingGroup);
    
    m_betAmountLabel = new QLabel("Amount: $0", this);
    m_betSlider = new QSlider(Qt::Horizontal, this);
    m_betSpinBox = new QSpinBox(this);
    
    m_betSlider->setMinimum(0);
    m_betSlider->setMaximum(1000);
    m_betSlider->setValue(0);
    
    m_betSpinBox->setMinimum(0);
    m_betSpinBox->setMaximum(10000);
    m_betSpinBox->setValue(0);
    
    m_bettingLayout->addWidget(m_betAmountLabel);
    m_bettingLayout->addWidget(m_betSlider);
    m_bettingLayout->addWidget(m_betSpinBox);
    
    // Initially disable betting controls
    m_bettingGroup->setEnabled(false);
}

void PokerTableWindow::connectSignals()
{
    connect(m_foldButton, &QPushButton::clicked, this, &PokerTableWindow::foldClicked);
    connect(m_callButton, &QPushButton::clicked, this, &PokerTableWindow::callClicked);
    connect(m_checkButton, &QPushButton::clicked, this, &PokerTableWindow::checkClicked);
    connect(m_betButton, &QPushButton::clicked, this, &PokerTableWindow::onRaiseAction);
    connect(m_raiseButton, &QPushButton::clicked, this, &PokerTableWindow::onRaiseAction);
    connect(m_allInButton, &QPushButton::clicked, this, &PokerTableWindow::allInClicked);
    
    // Connect betting controls
    connect(m_betSlider, &QSlider::valueChanged, this, &PokerTableWindow::onBetAmountChanged);
    connect(m_betSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &PokerTableWindow::onBetAmountChanged);
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
        // Fallback to colored rectangle
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
    if (seat < 0 || seat >= 9) return;
    
    auto& playerUI = m_playerComponents[seat];
    
    playerUI.nameLabel->setText(QString::fromStdString(player.getName()));
    playerUI.chipsLabel->setText(QString("$%1").arg(player.getCash()));
    
    // Show player area
    playerUI.playerGroup->setVisible(true);
    playerUI.playerGroup->setTitle(QString("Seat %1 - %2").arg(seat + 1).arg(QString::fromStdString(player.getName())));
}

void PokerTableWindow::showHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= 9) return;
    
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
    if (playerId < 0 || playerId >= 9) return;
    
    m_playerComponents[playerId].statusLabel->setText(status);
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
} // namespace pkt::ui::qtwidgets
