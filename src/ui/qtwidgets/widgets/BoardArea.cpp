// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

// BoardArea.cpp
#include "BoardArea.h"
#include <QPixmap>
#include <QDebug>

namespace pkt::ui::qtwidgets
{

BoardArea::BoardArea(QWidget* parent)
    : QGroupBox(parent)
    , m_layout(nullptr)
    , m_potLabel(nullptr)
    , m_roundStateLabel(nullptr)
{
    m_communityCards.fill(nullptr);
    setupUi();
}

void BoardArea::setupUi()
{
    // Center area styling
    setStyleSheet("QGroupBox {"
                  "  background: qradialgradient(cx: 0.5, cy: 0.5, radius: 1, "
                  "    stop: 0 rgba(255,255,255,0.8), stop: 0.7 rgba(248,249,250,0.6), stop: 1 rgba(233,236,239,0.4));"
                  "  border: 1px solid rgba(206,212,218,0.3);"
                  "  border-radius: 12px;"
                  "  margin: 0px;"
                  "  padding: 8px;"
                  "}");
    
    setFixedSize(360, 220);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(10);

    // Pot label
    m_potLabel = new QLabel("Pot: $0", this);
    m_potLabel->setAlignment(Qt::AlignCenter);
    m_potLabel->setStyleSheet("QLabel {"
                              "  color: #2c3e50;"
                              "  font-size: 24px;"
                              "  font-weight: 700;"
                              "  margin: 0px;"
                              "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                              "    stop: 0 rgba(255,255,255,0.9), stop: 1 rgba(248,249,250,0.8));"
                              "  border: 1px solid rgba(206,212,218,0.5);"
                              "  border-radius: 6px;"
                              "  padding: 10px 16px;"
                              "}");

    // Round state label
    m_roundStateLabel = new QLabel("Waiting for players...", this);
    m_roundStateLabel->setAlignment(Qt::AlignCenter);
    m_roundStateLabel->setStyleSheet("QLabel {"
                                     "  color: #6c757d;"
                                     "  font-size: 15px;"
                                     "  font-weight: 600;"
                                     "  margin: 0px;"
                                     "  background: rgba(248,249,250,0.7);"
                                     "  border: 1px solid rgba(206,212,218,0.3);"
                                     "  border-radius: 4px;"
                                     "  padding: 6px 12px;"
                                     "}");

    // Community cards layout
    auto communityLayout = new QHBoxLayout();
    communityLayout->setSpacing(4);
    
    for (int i = 0; i < 5; ++i)
    {
        m_communityCards[i] = new QLabel(this);
        m_communityCards[i]->setFixedSize(50, 70);
        m_communityCards[i]->setScaledContents(true);
        m_communityCards[i]->setStyleSheet("QLabel {"
                                           "  border: 2px solid #dee2e6;"
                                           "  border-radius: 8px;"
                                           "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                                           "    stop: 0 #ffffff, stop: 0.5 #fefefe, stop: 1 #f8f9fa);"
                                           "  margin: 0px;"
                                           "  padding: 2px;"
                                           "}");
        
        // Load card back image
        QPixmap cardBack(":/cards/flipside.png");
        if (cardBack.isNull())
        {
            cardBack = QPixmap(50, 70);
            cardBack.fill(Qt::blue);
        }
        m_communityCards[i]->setPixmap(cardBack);
        m_communityCards[i]->setVisible(true);
        
        communityLayout->addWidget(m_communityCards[i]);
    }

    // Add widgets to layout
    m_layout->addWidget(m_potLabel);
    m_layout->addSpacing(2);
    m_layout->addWidget(m_roundStateLabel);
    m_layout->addSpacing(12);
    m_layout->addLayout(communityLayout);
}

void BoardArea::updatePot(int amount)
{
    if (m_potLabel)
    {
        m_potLabel->setText(QString("Pot: $%1").arg(amount));
    }
}

void BoardArea::updateRoundState(pkt::core::GameState gameState)
{
    QString phaseText = gameStateToText(gameState);
    updateRoundState(QString("Phase: %1").arg(phaseText));
}

void BoardArea::updateRoundState(const QString& text)
{
    if (m_roundStateLabel)
    {
        m_roundStateLabel->setText(text);
    }
}

void BoardArea::showCommunityCards(const pkt::core::BoardCards& boardCards)
{
    qDebug() << "BoardArea::showCommunityCards called with numCards:" << boardCards.numCards;

    // Clear all community cards first
    clearCommunityCards();

    // Helper lambda to load card pixmap
    auto getCardPixmap = [](const pkt::core::Card& card) -> QPixmap {
        if (!card.isValid())
        {
            QPixmap fallback(50, 70);
            fallback.fill(Qt::white);
            return fallback;
        }
        
        QString imagePath = QString(":/cards/%1.png").arg(card.getIndex());
        QPixmap pixmap(imagePath);
        
        if (pixmap.isNull())
        {
            QPixmap fallback(50, 70);
            fallback.fill(Qt::white);
            return fallback;
        }
        
        return pixmap;
    };

    // Show cards based on number of cards dealt
    if (boardCards.numCards >= 3)
    {
        // Flop
        m_communityCards[0]->setPixmap(getCardPixmap(boardCards.flop1));
        m_communityCards[1]->setPixmap(getCardPixmap(boardCards.flop2));
        m_communityCards[2]->setPixmap(getCardPixmap(boardCards.flop3));
        m_communityCards[0]->setVisible(true);
        m_communityCards[1]->setVisible(true);
        m_communityCards[2]->setVisible(true);
    }
    if (boardCards.numCards >= 4)
    {
        // Turn
        m_communityCards[3]->setPixmap(getCardPixmap(boardCards.turn));
        m_communityCards[3]->setVisible(true);
    }
    if (boardCards.numCards >= 5)
    {
        // River
        m_communityCards[4]->setPixmap(getCardPixmap(boardCards.river));
        m_communityCards[4]->setVisible(true);
    }
}

void BoardArea::clearCommunityCards()
{
    for (int i = 0; i < 5; ++i)
    {
        if (m_communityCards[i])
        {
            m_communityCards[i]->clear();
            m_communityCards[i]->setVisible(false);
        }
    }
}

void BoardArea::reset()
{
    updatePot(0);
    updateRoundState("⏳ Starting new hand...");
    clearCommunityCards();
}

QString BoardArea::gameStateToText(pkt::core::GameState state) const
{
    switch (state)
    {
    case pkt::core::GameState::None:
        return "Waiting";
    case pkt::core::GameState::Preflop:
        return "Preflop";
    case pkt::core::GameState::Flop:
        return "Flop";
    case pkt::core::GameState::Turn:
        return "Turn";
    case pkt::core::GameState::River:
        return "River";
    case pkt::core::GameState::PostRiver:
        return "Showdown";
    default:
        return "Unknown";
    }
}

} // namespace pkt::ui::qtwidgets
