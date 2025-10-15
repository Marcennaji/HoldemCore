#include "PokerTableWindow.h"
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QPoint>
#include <QRegularExpression>
#include <QShowEvent>
#include <QSlider>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace pkt::ui::qtwidgets
{

// Action button styling constants
const QString btnStyle = "QPushButton {"
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
    : QWidget(parent), m_session(session), m_maxPlayers(6), m_activePlayerId(-1),
      m_dealerPosition(-1) // Default to 6 players
{
    if (!session)
    {
        throw std::invalid_argument("PokerTableWindow constructor: Session pointer cannot be null");
    }

    m_playerPanels.resize(m_maxPlayers, nullptr); // Will be created in createPlayerAreas
    m_cachedHoleCards.resize(m_maxPlayers);       // Cache dealt cards per player

    setWindowTitle("Poker Table - HoldemCore");
    setFixedSize(1200, 900); // Increased window size to prevent overlap between human cards and action buttons

    // Add try-catch around UI setup to catch any crashes
    try
    {
        setupUi();
        connectSignals();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            QString("PokerTableWindow constructor: Exception during UI setup: %1").arg(e.what()).toStdString());
    }
}

void PokerTableWindow::initializeWithGameData(const pkt::core::GameData& gameData)
{
    // Store the start money for access by other components
    m_startMoney = gameData.startMoney;

    // Update max players and resize components if needed
    if (gameData.maxNumberOfPlayers != m_maxPlayers)
    {
        m_maxPlayers = gameData.maxNumberOfPlayers;

        // Clear existing player panels
        for (auto* panel : m_playerPanels)
        {
            if (panel)
            {
                delete panel;
            }
        }

        // Resize and recreate ALL player panels (including human player)
        m_playerPanels.clear();
        m_playerPanels.resize(m_maxPlayers, nullptr);
        m_cachedHoleCards.clear();
        m_cachedHoleCards.resize(m_maxPlayers);

        // Recreate UI with new player count
        createPlayerAreas();

        // Position all players in circular layout
        positionPlayersInCircle();
    }

    for (int seat = 0; seat < m_maxPlayers; ++seat)
    {
        updatePlayerCash(seat, gameData.startMoney);
    }
}

void PokerTableWindow::setupUi()
{
    // Create the main layout
    m_mainLayout = new QVBoxLayout(this);

    // Create all UI components properly
    createPlayerAreas();

    // Create board area
    m_boardArea = new BoardArea(this);
    positionBoardArea();

    // Create action bar
    m_actionBar = new ActionBar(this);

    // Create Next Hand button
    m_nextHandButton = new QPushButton("Next Hand", this);
    m_nextHandButton->setStyleSheet("QPushButton {"
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
                                    "}");
    m_nextHandButton->setVisible(false); // Hidden by default

    // Add all player panels to the main widget (they will be positioned absolutely)
    for (auto* panel : m_playerPanels)
    {
        if (panel)
        {
            panel->show();
        }
    }

    // Position all players in circular layout (will be called again in showEvent)
    positionPlayersInCircle();

    // Add stretch space to push action controls to bottom
    m_mainLayout->addStretch();

    // Add action bar
    m_mainLayout->addWidget(m_actionBar);

    // Add Next Hand button (centered)
    auto nextHandLayout = new QHBoxLayout();
    nextHandLayout->addStretch();
    nextHandLayout->addWidget(m_nextHandButton);
    nextHandLayout->addStretch();
    m_mainLayout->addLayout(nextHandLayout);

    // Set layout and style
    this->setLayout(m_mainLayout);

    // Elegant light table background styling
    this->setStyleSheet("PokerTableWindow {"
                        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                        "    stop: 0 #f8f9fa, stop: 0.5 #e9ecef, stop: 1 #ced4da);"
                        "  border: 2px solid #6c757d;"
                        "  border-radius: 12px;"
                        "}"
                        "QGroupBox {"
                        "  border-radius: 6px;"
                        "  font-weight: normal;"
                        "}");

    this->resize(1200, 800);

    // Do not show the window here; caller controls visibility to avoid flicker
}

void PokerTableWindow::closeEvent(QCloseEvent* event)
{
    // Prevent widget destruction; hide and notify controller/StartWindow
    event->accept();
    this->hide();
    emit windowClosed();
}

void PokerTableWindow::createPlayerAreas()
{
    // Create PlayerPanel widgets for ALL players (including human player at index 0)
    for (int i = 0; i < m_maxPlayers; ++i)
    {
        bool isHuman = (i == 0);
        m_playerPanels[i] = new PlayerPanel(i, isHuman, this);
        m_playerPanels[i]->setVisible(true);
    }
}

void PokerTableWindow::connectSignals()
{
    // Connect ActionBar signals to window signals
    connect(m_actionBar, &ActionBar::foldClicked, this, &PokerTableWindow::foldClicked);
    connect(m_actionBar, &ActionBar::callClicked, this, &PokerTableWindow::callClicked);
    connect(m_actionBar, &ActionBar::checkClicked, this, &PokerTableWindow::checkClicked);
    connect(m_actionBar, &ActionBar::allInClicked, this, &PokerTableWindow::allInClicked);
    connect(m_actionBar, &ActionBar::betClicked, this, &PokerTableWindow::onBetAction);
    connect(m_actionBar, &ActionBar::raiseClicked, this, &PokerTableWindow::onRaiseAction);

    // Reset bet controls after simple actions
    connect(m_actionBar, &ActionBar::foldClicked, m_actionBar, &ActionBar::resetBetControls);
    connect(m_actionBar, &ActionBar::callClicked, m_actionBar, &ActionBar::resetBetControls);
    connect(m_actionBar, &ActionBar::checkClicked, m_actionBar, &ActionBar::resetBetControls);
    connect(m_actionBar, &ActionBar::allInClicked, m_actionBar, &ActionBar::resetBetControls);

    // Connect Next Hand button
    connect(m_nextHandButton, &QPushButton::clicked, this, &PokerTableWindow::onNextHandClicked);
}

// Slot implementations
void PokerTableWindow::onBetAction()
{
    // Get the bet amount from ActionBar
    int betAmount = m_actionBar->getBetValue();
    emit betClicked(betAmount);
    m_actionBar->resetBetControls();
}

void PokerTableWindow::onRaiseAction()
{
    // Get the bet amount from ActionBar
    int raiseAmount = m_actionBar->getBetValue();
    emit raiseClicked(raiseAmount);
    m_actionBar->resetBetControls();
}

void PokerTableWindow::onNextHandClicked()
{
    // Hide the Next Hand button and show action bar
    m_nextHandButton->setVisible(false);
    if (m_actionBar)
    {
        m_actionBar->setVisible(true);
    }

    emit nextHandRequested();
}

// Card visualization helpers
QPixmap PokerTableWindow::getCardPixmap(const pkt::core::Card& card) const
{
    if (!card.isValid())
    {
        return getCardBackPixmap();
    }

    QString imagePath = getCardImagePath(card.getIndex());
    QPixmap pixmap(imagePath);

    if (pixmap.isNull())
    {
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

    if (pixmap.isNull())
    {
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
    if (m_boardArea)
    {
        m_boardArea->updatePot(amount);
    }
}

void PokerTableWindow::refreshPlayer(int seat, const PlayerDisplayInfo& playerInfo)
{
    if (seat < 0 || seat >= m_maxPlayers || !m_playerPanels[seat])
        return;

    // Show player panel and update with strategy name
    m_playerPanels[seat]->setVisible(true);

    // Convert DTO data to Qt strings
    QString strategyName = QString::fromStdString(playerInfo.strategyName);
    QString playerName = QString::fromStdString(playerInfo.playerName);

    m_playerPanels[seat]->updatePlayerInfo(playerName, strategyName, playerInfo.chips);
}

void PokerTableWindow::updatePlayerCash(unsigned playerId, int newChips)
{
    if (playerId >= static_cast<unsigned>(m_playerPanels.size()) || !m_playerPanels[playerId])
        return;

    m_playerPanels[playerId]->updateChips(newChips);
}

void PokerTableWindow::showHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers || !m_playerPanels[seat])
        return;

    if (holeCards.isValid())
    {
        m_playerPanels[seat]->showHoleCards(holeCards);
        // Cache for potential showdown reveal
        cacheHoleCards(seat, holeCards);
    }
    else
    {
        m_playerPanels[seat]->hideHoleCards();
    }
}

void PokerTableWindow::showBoardCards(const pkt::core::BoardCards& boardCards)
{
    qDebug() << "showBoardCards called with numCards:" << boardCards.numCards;

    if (m_boardArea)
    {
        m_boardArea->showCommunityCards(boardCards);
    }

    // Track river for showdown inference
    if (boardCards.numCards >= 5)
    {
        m_sawRiver = true;
    }
}

void PokerTableWindow::updateGamePhase(pkt::core::GameState gameState)
{
    QString phaseText;
    switch (gameState)
    {
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

    // Update the board area round state
    if (m_boardArea)
    {
        m_boardArea->updateRoundState(phaseText);
    }
}

void PokerTableWindow::clearActionLabelsForNewRound()
{
    for (auto* panel : m_playerPanels)
    {
        // Keep 'folded' text visible across rounds; clear others
        if (panel && !panel->isFolded())
        {
            panel->clearAction();
        }
    }
}

void PokerTableWindow::clearPlayerActionLabel(int playerId)
{
    if (playerId < 0 || playerId >= m_maxPlayers || !m_playerPanels[playerId])
        return;

    m_playerPanels[playerId]->clearAction();
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
    if (!m_actionBar)
        return;

    // Check which actions are available
    bool canFold = false, canCall = false, canCheck = false;
    bool canBet = false, canRaise = false, canAllIn = false;

    for (const auto& action : actions)
    {
        switch (action)
        {
        case pkt::core::ActionType::Fold:
            canFold = true;
            break;
        case pkt::core::ActionType::Call:
            canCall = true;
            break;
        case pkt::core::ActionType::Check:
            canCheck = true;
            break;
        case pkt::core::ActionType::Bet:
            canBet = true;
            break;
        case pkt::core::ActionType::Raise:
            canRaise = true;
            break;
        case pkt::core::ActionType::Allin:
            canAllIn = true;
            break;
        default:
            break;
        }
    }

    m_actionBar->setAvailableActions(canFold, canCall, canCheck, canBet, canRaise, canAllIn);
}

void PokerTableWindow::enablePlayerInput(bool enabled)
{
    if (m_actionBar)
    {
        m_actionBar->enableInput(enabled);
    }
}

// Structured UI updates (no string parsing)
void PokerTableWindow::showPlayerAction(int playerId, pkt::core::ActionType action, int amount)
{
    if (playerId < 0 || playerId >= m_maxPlayers || !m_playerPanels[playerId])
        return;

    m_playerPanels[playerId]->showAction(action, amount);
}

void PokerTableWindow::showPlayerTurn(int playerId)
{
    if (playerId < 0 || playerId >= m_maxPlayers)
        return;
    // Do NOT overwrite the player's last action during the betting round.
    // We only indicate turn via highlighting to keep last action visible.
    setActivePlayer(playerId);
}

void PokerTableWindow::onHandCompleted()
{
    // Show the Next Hand button when a hand is completed
    m_nextHandButton->setVisible(true);

    // Hide action bar during hand completion
    if (m_actionBar)
    {
        m_actionBar->setVisible(false);
    }
}

void PokerTableWindow::resetForNewHand()
{
    qDebug() << "Resetting UI for new hand - clearing community cards";

    // Hide the Next Hand button
    m_nextHandButton->setVisible(false);

    // Show action bar for the new hand
    if (m_actionBar)
    {
        m_actionBar->setVisible(true);
    }

    // Explicitly clear board cards using the same method as game events
    pkt::core::BoardCards emptyBoard; // Default constructor creates preflop state (0 cards)
    showBoardCards(emptyBoard);

    // Reset all player panels
    for (auto* panel : m_playerPanels)
    {
        if (panel)
        {
            panel->reset();
        }
    }

    // Reset showdown flags and caches
    m_reachedShowdown = false;
    m_sawRiver = false;
    if (!m_cachedHoleCards.empty())
    {
        for (auto& hc : m_cachedHoleCards)
        {
            hc.reset();
        }
    }

    // Clear player highlights
    clearPlayerHighlights();

    // Hide dealer indicators
    m_dealerPosition = -1;

    // Reset pot display
    refreshPot(0);

    // Update round state
    if (m_boardArea)
    {
        m_boardArea->updateRoundState("⏳ Starting new hand...");
    }

    // Enable player input for new hand
    enablePlayerInput(true);
}

void PokerTableWindow::showWinners(const std::list<unsigned>& winnerIds, int totalPot)
{
    // Show a winner badge over each winner's area. If multiple, display all.
    const bool isSplit = winnerIds.size() > 1;
    for (unsigned id : winnerIds)
    {
        if (id < m_playerPanels.size() && m_playerPanels[id])
        {
            m_playerPanels[id]->showWinner(true);

            // If we reached showdown (not everyone folded), reveal the winner's hole cards
            if ((m_reachedShowdown || m_sawRiver || isSplit) && id < m_cachedHoleCards.size())
            {
                const auto& hc = m_cachedHoleCards[id];
                if (hc.isValid())
                {
                    showHoleCards(static_cast<int>(id), hc);
                }
            }
        }
    }
}

void PokerTableWindow::revealShowdownOrder(const std::vector<unsigned>& revealOrder)
{
    // Reveal strictly according to engine-provided order. Use cached hole cards.
    for (unsigned id : revealOrder)
    {
        if (id < m_playerPanels.size() && m_playerPanels[id] && id < m_cachedHoleCards.size())
        {
            const auto& hc = m_cachedHoleCards[id];
            if (!m_playerPanels[id]->isFolded() && hc.isValid())
            {
                showHoleCards(static_cast<int>(id), hc);
            }
        }
        // slight UI pacing to make reveal readable
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
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

    // Clearance from the board area (community cards) in both axes
    const int centerHalfH = m_boardArea ? (m_boardArea->height() / 2) : 90;
    const int centerHalfW = m_boardArea ? (m_boardArea->width() / 2) : 180;
    const int minVerticalRadius = centerHalfH + widgetHalfH + 32;
    const int minHorizontalRadius = centerHalfW + widgetHalfW + 32;

    // Edge margins
    const int edgeMargin = 16;

    // Maximum radius constrained by available width/height (full height for larger circle)
    const int maxRadiusByHeight = std::max(0, fullHeight / 2 - widgetHalfH - edgeMargin);
    const int maxRadiusByWidth = std::max(0, width() / 2 - widgetHalfW - edgeMargin);
    int radius = std::min({maxRadiusByHeight, maxRadiusByWidth});

    // Ensure minimal clearance from center area
    const int desiredMin = std::max(minVerticalRadius, minHorizontalRadius);
    radius = std::max(radius, desiredMin);

    // Hard cap to avoid off-screen placement
    const int hardCap = std::min(width(), fullHeight) / 2 - std::max(widgetHalfH, widgetHalfW) - edgeMargin;
    radius = std::min(radius, std::max(0, hardCap));

    // Reduce the radius to 90% of current while respecting minimum clearance
    const double radiusScale = 0.90;
    radius = static_cast<int>(std::round(radius * radiusScale));
    if (radius < desiredMin)
        radius = desiredMin;

    // Clamp center Y so bottom-most player stays above the reserved controls area (with extra padding)
    const int bottomExtraPadding = 32; // Additional breathing room above action controls
    const int allowedBottom =
        fullHeight - bottomReserve - edgeMargin - bottomExtraPadding; // y for bottom edge of player widget
    const int maxCy = allowedBottom - radius - widgetHalfH;           // cy + radius + widgetHalfH <= allowedBottom
    const int minCy = edgeMargin + radius + widgetHalfH;              // top-most player not clipped
    int cy = std::clamp(center.y(), minCy, std::max(minCy, maxCy));
    center.setY(cy);

    // Position all player panels in a circle
    for (int i = 0; i < m_maxPlayers; ++i)
    {
        auto* panel = m_playerPanels[i];
        if (!panel)
            continue;

        QPoint position = calculateCircularPosition(i, m_maxPlayers, center, radius);
        panel->move(position);
        panel->show();  // Ensure widget is visible
        panel->raise(); // Bring to front
    }

    // After moving player groups, reposition dealer badges to sit on the border
    positionDealerButtons();
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

void PokerTableWindow::positionBoardArea()
{
    if (m_boardArea)
    {
        const int bottomReserve = reservedBottomHeight();
        const int effectiveHeight = std::max(0, height() - bottomReserve);
        int centerX = (width() - m_boardArea->width()) / 2;

        // Move the board area lower in the window by adding an offset
        // Instead of centering it vertically, position it at about 60% down from the top
        int baseY = (effectiveHeight - m_boardArea->height()) / 2;
        int lowerOffset = effectiveHeight / 8; // Move down by 1/8 of the effective height
        int centerY = baseY + lowerOffset;

        // Ensure it doesn't go too low and overlap with the bottom controls
        int maxY = effectiveHeight - m_boardArea->height() - 20; // 20px margin from bottom
        centerY = std::min(centerY, maxY);

        m_boardArea->move(centerX, centerY);
        // Ensure the board area renders above players if close
        m_boardArea->raise();
    }
}

void PokerTableWindow::positionDealerButtons()
{
    // Place the dealer "D" badge at the bottom-right corner, slightly overlapping the panel border
    for (auto* panel : m_playerPanels)
    {
        if (!panel || !panel->dealerButton() || !panel->dealerButton()->isVisible())
            continue;

        QLabel* badge = panel->dealerButton();
        // Coordinates relative to parent (PokerTableWindow)
        const QPoint panelTopLeft = panel->pos();
        const int badgeW = badge->width();
        const int badgeH = badge->height();
        // Bottom-right corner with a small overlap
        const int x = panelTopLeft.x() + panel->width() - badgeW + 4;  // overlap 4px outside on X
        const int y = panelTopLeft.y() + panel->height() - badgeH + 4; // overlap 4px outside on Y
        badge->move(x, y);
        badge->raise();
    }
}

void PokerTableWindow::cacheHoleCards(int seat, const pkt::core::HoleCards& holeCards)
{
    if (seat < 0 || seat >= m_maxPlayers)
        return;
    if (static_cast<size_t>(seat) >= m_cachedHoleCards.size())
    {
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

    // Since window size is now fixed, repositioning should be consistent
    // Keep the repositioning logic for showEvent compatibility
    positionPlayersInCircle();
    positionBoardArea();
    positionDealerButtons();
}

void PokerTableWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // Position elements when window is first shown (now we have proper size)
    positionPlayersInCircle();
    positionBoardArea();
    positionDealerButtons();
}

int PokerTableWindow::reservedBottomHeight() const
{
    // Reserve enough vertical space to fit: action bar and next hand button rows.
    // If widgets exist, use their size hints; otherwise fall back to conservative defaults.
    int actionBarH = m_actionBar ? m_actionBar->sizeHint().height() : 100;
    int nextHandH = m_nextHandButton ? (m_nextHandButton->sizeHint().height() + 12) : 40;
    // Include some padding and layout spacing
    int padding = 24; // extra gap between human cards and controls
    return actionBarH + nextHandH + padding;
}

// Player state indicator implementations
void PokerTableWindow::setActivePlayer(int playerId)
{
    // Clear previous active player highlighting
    clearPlayerHighlights();

    m_activePlayerId = playerId;

    // Highlight the active player
    if (playerId >= 0 && playerId < static_cast<int>(m_playerPanels.size()) && m_playerPanels[playerId])
    {
        m_playerPanels[playerId]->setActive(true);
    }
}

void PokerTableWindow::setDealerPosition(int playerId)
{
    // Hide all dealer buttons first
    for (auto* panel : m_playerPanels)
    {
        if (panel)
        {
            panel->setDealer(false);
        }
    }

    m_dealerPosition = playerId;

    // Show dealer button for the specified player
    if (playerId >= 0 && playerId < static_cast<int>(m_playerPanels.size()) && m_playerPanels[playerId])
    {
        m_playerPanels[playerId]->setDealer(true);
        // Immediately position it over the border
        positionDealerButtons();
    }
}

void PokerTableWindow::setDealerFromSmallBlind(int smallBlindId)
{
    if (smallBlindId < 0 || smallBlindId >= m_maxPlayers)
        return;
    // Heads-up: Button is also Small Blind
    int dealerId = smallBlindId;
    if (m_maxPlayers > 2)
    {
        // 3+ players: Dealer is immediately to the right of Small Blind (previous seat modulo N)
        dealerId = smallBlindId - 1;
        if (dealerId < 0)
            dealerId = m_maxPlayers - 1;
    }
    setDealerPosition(dealerId);
}

void PokerTableWindow::clearPlayerHighlights()
{
    // Reset all players to their default styling
    for (auto* panel : m_playerPanels)
    {
        if (panel)
        {
            panel->setActive(false);
        }
    }

    m_activePlayerId = -1;
}

void PokerTableWindow::updatePlayerStateIndicators()
{
    // Refresh the current active player highlighting
    if (m_activePlayerId >= 0)
    {
        int currentActive = m_activePlayerId;
        clearPlayerHighlights();
        setActivePlayer(currentActive);
    }

    // Refresh dealer button
    if (m_dealerPosition >= 0)
    {
        setDealerPosition(m_dealerPosition);
    }
}

} // namespace pkt::ui::qtwidgets
