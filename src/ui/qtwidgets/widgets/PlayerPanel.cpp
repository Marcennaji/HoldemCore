// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

// PlayerPanel.cpp
#include "PlayerPanel.h"
#include <QPixmap>

namespace pkt::ui::qtwidgets
{

PlayerPanel::PlayerPanel(int playerId, bool isHuman, QWidget* parent)
    : QGroupBox(parent)
    , m_playerId(playerId)
    , m_isHuman(isHuman)
    , m_playerName(isHuman ? "You" : QString("Bot %1").arg(playerId))
    , m_currentChips(0)
    , m_isFolded(false)
    , m_isActive(false)
    , m_layout(nullptr)
    , m_holeCard1(nullptr)
    , m_holeCard2(nullptr)
    , m_currentActionLabel(nullptr)
    , m_winnerLabel(nullptr)
    , m_dealerButton(nullptr)
    , m_card1OpacityEffect(nullptr)
    , m_card2OpacityEffect(nullptr)
{
    setupUi();
}

void PlayerPanel::setupUi()
{
    setTitle(m_playerName);
    setStyleSheet(defaultPlayerGroupStyle());
    setFixedSize(120, 136);

    m_layout = new QVBoxLayout(this);

    // Hole cards in horizontal layout
    auto cardLayout = new QHBoxLayout();
    m_holeCard1 = new QLabel(this);
    m_holeCard2 = new QLabel(this);

    // Unified card styling and size
    m_holeCard1->setFixedSize(50, 70);
    m_holeCard2->setFixedSize(50, 70);
    QString cardStyle = "QLabel {"
                        "  border: 1px solid #ced4da;"
                        "  border-radius: 6px;"
                        "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                        "    stop: 0 #ffffff, stop: 1 #f8f9fa);"
                        "  margin: 1px;"
                        "}";
    m_holeCard1->setStyleSheet(cardStyle);
    m_holeCard2->setStyleSheet(cardStyle);

    m_holeCard1->setScaledContents(true);
    m_holeCard2->setScaledContents(true);

    // Initialize with card backs
    showCardBack();

    // Prepare opacity effects for fold visualization
    m_card1OpacityEffect = new QGraphicsOpacityEffect(this);
    m_card2OpacityEffect = new QGraphicsOpacityEffect(this);
    m_card1OpacityEffect->setOpacity(1.0);
    m_card2OpacityEffect->setOpacity(1.0);
    m_holeCard1->setGraphicsEffect(m_card1OpacityEffect);
    m_holeCard2->setGraphicsEffect(m_card2OpacityEffect);

    cardLayout->addWidget(m_holeCard1);
    cardLayout->addWidget(m_holeCard2);

    // Current action label
    m_currentActionLabel = new QLabel("", this);
    m_currentActionLabel->setAlignment(Qt::AlignCenter);
    m_currentActionLabel->setStyleSheet(currentActionLabelStyleBase());

    // Winner badge label (hidden by default)
    m_winnerLabel = new QLabel("WINNER", this);
    m_winnerLabel->setAlignment(Qt::AlignCenter);
    m_winnerLabel->setStyleSheet("QLabel {"
                                  "  color: #155724;"
                                  "  background: rgba(212, 237, 218, 0.95);"
                                  "  border: 1px solid #c3e6cb;"
                                  "  border-radius: 6px;"
                                  "  font-weight: 800;"
                                  "  padding: 2px 4px;"
                                  "}");
    m_winnerLabel->setVisible(false);

    // Create dealer button indicator (positioned by parent, not in layout)
    m_dealerButton = new QLabel("D", parentWidget());
    m_dealerButton->setFixedSize(18, 18);
    m_dealerButton->setAlignment(Qt::AlignCenter);
    m_dealerButton->setStyleSheet("QLabel {"
                                   "  background: rgba(255, 215, 0, 0.85);"
                                   "  border: 1px solid #b8860b;"
                                   "  border-radius: 9px;"
                                   "  color: #202020;"
                                   "  font-weight: 600;"
                                   "  font-size: 10px;"
                                   "}");
    m_dealerButton->setVisible(false);

    m_layout->addLayout(cardLayout);
    m_layout->addWidget(m_currentActionLabel);
    m_layout->addWidget(m_winnerLabel);
}

void PlayerPanel::updatePlayerInfo(const QString& name, int chips)
{
    m_playerName = name;
    m_currentChips = chips;
    setTitle(QString("%1  chips = %2").arg(m_playerName).arg(m_currentChips));
}

void PlayerPanel::updateChips(int chips)
{
    m_currentChips = chips;
    QString prefix = m_isHuman ? "You" : QString("Bot %1").arg(m_playerId);
    setTitle(QString("%1  chips = %2").arg(prefix).arg(m_currentChips));
}

void PlayerPanel::showHoleCards(const pkt::core::HoleCards& cards)
{
    if (!cards.isValid() || m_isFolded)
    {
        showCardBack();
        return;
    }

    // Load card images from resources
    QString card1Path = QString(":/cards/%1.png").arg(cards.card1.getIndex());
    QString card2Path = QString(":/cards/%1.png").arg(cards.card2.getIndex());

    QPixmap pixmap1(card1Path);
    QPixmap pixmap2(card2Path);

    // Fallback to white rectangle if image fails to load
    if (pixmap1.isNull())
    {
        pixmap1 = QPixmap(50, 70);
        pixmap1.fill(Qt::white);
    }
    if (pixmap2.isNull())
    {
        pixmap2 = QPixmap(50, 70);
        pixmap2.fill(Qt::white);
    }

    m_holeCard1->setPixmap(pixmap1);
    m_holeCard2->setPixmap(pixmap2);
}

void PlayerPanel::hideHoleCards()
{
    showCardBack();
}

void PlayerPanel::showCardBack()
{
    QPixmap cardBack(":/cards/flipside.png");

    if (cardBack.isNull())
    {
        // Fallback to blue rectangle if resource loading fails
        cardBack = QPixmap(50, 70);
        cardBack.fill(Qt::blue);
    }

    m_holeCard1->setPixmap(cardBack);
    m_holeCard2->setPixmap(cardBack);
}

void PlayerPanel::showAction(pkt::core::ActionType action, int amount)
{
    QString text;
    switch (action)
    {
    case pkt::core::ActionType::Fold:
        text = "folded";
        setFolded(true);
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
        text = QString::fromUtf8(pkt::core::actionTypeToString(action)).toUpper();
        break;
    }

    m_currentActionLabel->setText(text);
    m_currentActionLabel->setStyleSheet(currentActionLabelStyleFor(text));
}

void PlayerPanel::clearAction()
{
    m_currentActionLabel->setText("");
    m_currentActionLabel->setStyleSheet(currentActionLabelStyleBase());
}

void PlayerPanel::setActive(bool active)
{
    m_isActive = active;
    setStyleSheet(active ? activePlayerGroupStyle() : defaultPlayerGroupStyle());
}

void PlayerPanel::setDealer(bool isDealer)
{
    m_dealerButton->setVisible(isDealer);
}

void PlayerPanel::setFolded(bool folded)
{
    m_isFolded = folded;
    applyFoldVisual();
}

void PlayerPanel::showWinner(bool isWinner)
{
    m_winnerLabel->setVisible(isWinner);
}

void PlayerPanel::reset()
{
    m_isFolded = false;
    m_isActive = false;
    clearAction();
    showCardBack();
    clearFoldVisual();
    m_winnerLabel->setVisible(false);
    m_dealerButton->setVisible(false);
    setStyleSheet(defaultPlayerGroupStyle());
}

void PlayerPanel::applyFoldVisual()
{
    if (m_card1OpacityEffect && m_card2OpacityEffect)
    {
        m_card1OpacityEffect->setOpacity(m_isFolded ? 0.35 : 1.0);
        m_card2OpacityEffect->setOpacity(m_isFolded ? 0.35 : 1.0);
    }
    if (m_isFolded)
    {
        showCardBack();
    }
}

void PlayerPanel::clearFoldVisual()
{
    if (m_card1OpacityEffect && m_card2OpacityEffect)
    {
        m_card1OpacityEffect->setOpacity(1.0);
        m_card2OpacityEffect->setOpacity(1.0);
    }
}

QString PlayerPanel::currentActionLabelStyleBase() const
{
    return QString("QLabel {"
                   "  color: #1f3044;"
                   "  font-size: 14px;"
                   "  font-weight: 700;"
                   "  border: 1px solid #cbd5e1;"
                   "  border-radius: 6px;"
                   "  background: rgba(255,255,255,0.9);"
                   "  padding: 4px 6px;"
                   "}");
}

QString PlayerPanel::currentActionLabelStyleFor(const QString& action) const
{
    QString base = currentActionLabelStyleBase();
    
    // For folds, keep neutral styling but make it italic and not bold
    if (action.compare("folded", Qt::CaseInsensitive) == 0 || action.compare("FOLD", Qt::CaseInsensitive) == 0)
    {
        return QString("QLabel {"
                       "  color: #1f3044;"
                       "  font-size: 14px;"
                       "  font-style: italic;"
                       "  font-weight: 400;"
                       "  border: 1px solid #cbd5e1;"
                       "  border-radius: 6px;"
                       "  background: rgba(255,255,255,0.9);"
                       "  padding: 4px 6px;"
                       "}");
    }
    if (action.compare("CALL", Qt::CaseInsensitive) == 0 || action.compare("CHECK", Qt::CaseInsensitive) == 0)
    {
        return base + " QLabel { color: #1b5e20; border-color: #c8e6c9; background: #e8f5e9; }";
    }
    if (action.compare("BET", Qt::CaseInsensitive) == 0 || action.compare("RAISE", Qt::CaseInsensitive) == 0 ||
        action.contains("ALL", Qt::CaseInsensitive))
    {
        return base + " QLabel { color: #0d47a1; border-color: #bbdefb; background: #e3f2fd; }";
    }
    return base;
}

QString PlayerPanel::defaultPlayerGroupStyle() const
{
    return QString("QGroupBox {"
                   "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                   "    stop: 0 #ffffff, stop: 0.5 #f8f9fa, stop: 1 #e9ecef);"
                   "  border: 2px solid #dee2e6;"
                   "  border-radius: 10px;"
                   "  margin-top: 8px;"
                   "  color: #495057;"
                   "  font-weight: 600;"
                   "  font-size: 12px;"
                   "}"
                   "QGroupBox:title {"
                   "  subcontrol-origin: margin;"
                   "  left: 8px;"
                   "  padding: 0 4px 0 4px;"
                   "}");
}

QString PlayerPanel::activePlayerGroupStyle() const
{
    return QString("QGroupBox {"
                   "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                   "    stop: 0 #e8f4fd, stop: 0.5 #e3f2fd, stop: 1 #bbdefb);"
                   "  border: 3px solid #1976d2;"
                   "  border-radius: 12px;"
                   "  margin-top: 8px;"
                   "  color: #0d47a1;"
                   "  font-weight: bold;"
                   "}"
                   "QGroupBox:title {"
                   "  subcontrol-origin: margin;"
                   "  left: 8px;"
                   "  padding: 0 4px 0 4px;"
                   "  color: #0d47a1;"
                   "  font-weight: bold;"
                   "}");
}

} // namespace pkt::ui::qtwidgets
