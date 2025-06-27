// PokerTableWindow.h
#pragma once

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <array>
#include <memory>

#include "core/engine/model/GameState.h"
#include "core/player/Player.h"

namespace pkt::core
{
class Session;
class IBoard;
} // namespace pkt::core

class PokerTableWindow : public QWidget
{
    Q_OBJECT

  public:
    explicit PokerTableWindow(std::shared_ptr<pkt::core::Session> session, QWidget* parent = nullptr);
    ~PokerTableWindow() override = default;

    void refresh();
    void refreshPot(int amount);
    void refreshPlayer(int seat, const pkt::core::player::Player& player);
    void showHoleCards(int seat, int card1, int card2);
    void showBoardCards(const std::array<int, 5>& cards);

  signals:
    void playerActionRequested();
    void betClicked(int amount);
    void foldClicked();
    void callClicked();
    void checkClicked();
    void allInClicked();

  private:
    void setupUi();
    void connectSignals();

    std::shared_ptr<pkt::core::Session> m_session;
    QGridLayout* m_layout;
    QLabel* m_potLabel;
    std::array<QLabel*, 9> m_playerLabels;
    std::array<QLabel*, 2> m_holeCardLabels;
    std::array<QLabel*, 5> m_boardCardLabels;

    QPushButton* m_foldButton;
    QPushButton* m_callButton;
    QPushButton* m_checkButton;
    QPushButton* m_betButton;
    QPushButton* m_allInButton;

    void createPlayerAreas();
    void createBoardArea();
    void createActionButtons();
};
