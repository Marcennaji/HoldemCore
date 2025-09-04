#include "PokerTableWindow.h"
#include <QString>
#include <QVBoxLayout>

namespace pkt::ui::qtwidgets
{

PokerTableWindow::PokerTableWindow(pkt::core::Session* session, QWidget* parent) : QWidget(parent), m_session(session)
{
    setupUi();
    connectSignals();
}

void PokerTableWindow::setupUi()
{
    m_layout = new QGridLayout(this);

    m_potLabel = new QLabel("Pot: 0", this);
    m_layout->addWidget(m_potLabel, 0, 1, Qt::AlignCenter);

    createPlayerAreas();
    createBoardArea();
    createActionButtons();
}

void PokerTableWindow::createPlayerAreas()
{
    for (int i = 0; i < m_playerLabels.size(); ++i)
    {
        m_playerLabels[i] = new QLabel(QString("Seat %1").arg(i + 1), this);
        m_layout->addWidget(m_playerLabels[i], (i < 5) ? 1 : 3, (i < 5) ? i : i - 5);
    }
}

void PokerTableWindow::createBoardArea()
{
    for (int i = 0; i < m_boardCardLabels.size(); ++i)
    {
        m_boardCardLabels[i] = new QLabel("[Card]", this);
        m_layout->addWidget(m_boardCardLabels[i], 2, i);
    }
    for (int i = 0; i < m_holeCardLabels.size(); ++i)
    {
        m_holeCardLabels[i] = new QLabel("[HC]", this);
        m_layout->addWidget(m_holeCardLabels[i], 4, i);
    }
}

void PokerTableWindow::createActionButtons()
{
    m_foldButton = new QPushButton("Fold", this);
    m_callButton = new QPushButton("Call", this);
    m_checkButton = new QPushButton("Check", this);
    m_betButton = new QPushButton("Bet", this);
    m_allInButton = new QPushButton("All-In", this);

    m_layout->addWidget(m_foldButton, 5, 0);
    m_layout->addWidget(m_callButton, 5, 1);
    m_layout->addWidget(m_checkButton, 5, 2);
    m_layout->addWidget(m_betButton, 5, 3);
    m_layout->addWidget(m_allInButton, 5, 4);
}

void PokerTableWindow::connectSignals()
{
    connect(m_foldButton, &QPushButton::clicked, this, &PokerTableWindow::foldClicked);
    connect(m_callButton, &QPushButton::clicked, this, &PokerTableWindow::callClicked);
    connect(m_checkButton, &QPushButton::clicked, this, &PokerTableWindow::checkClicked);
    connect(m_betButton, &QPushButton::clicked, [this]() { emit betClicked(100); });
    connect(m_allInButton, &QPushButton::clicked, this, &PokerTableWindow::allInClicked);
}

void PokerTableWindow::refresh()
{
    // Placeholder: refresh players, pot, etc.
}

void PokerTableWindow::refreshPot(int amount)
{
    m_potLabel->setText(QString("Pot: %1").arg(amount));
}

void PokerTableWindow::refreshPlayer(int seat, const pkt::core::player::Player& player)
{
    if (seat < 0 || seat >= static_cast<int>(m_playerLabels.size()))
        return;
    m_playerLabels[seat]->setText(
        QString("%1 ($%2)").arg(QString::fromStdString(player.getName())).arg(player.getCash()));
}

void PokerTableWindow::showHoleCards(int seat, int card1, int card2)
{
    m_holeCardLabels[0]->setText(QString("[%1]").arg(card1));
    m_holeCardLabels[1]->setText(QString("[%1]").arg(card2));
}

void PokerTableWindow::showBoardCards(const std::array<int, 5>& cards)
{
    for (int i = 0; i < cards.size(); ++i)
    {
        m_boardCardLabels[i]->setText(QString("[%1]").arg(cards[i]));
    }
}
} // namespace pkt::ui::qtwidgets
