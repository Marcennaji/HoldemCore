#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>

namespace pkt::ui::qtwidgets
{

/**
 * @brief ActionBar widget for poker game actions and betting controls
 * 
 * Manages the human player input controls including action buttons (fold, call, check,
 * bet, raise, all-in) and betting amount controls (slider, spinbox).
 */
class ActionBar : public QWidget
{
    Q_OBJECT

  public:
    explicit ActionBar(QWidget* parent = nullptr);

    // Action button control
    void setAvailableActions(bool canFold, bool canCall, bool canCheck,
                            bool canBet, bool canRaise, bool canAllIn);
    void updateCallAmount(int amount);
    void updateRaiseAmount(int minRaise, int maxRaise);
    void enableInput(bool enabled);
    
    // Betting controls
    void setBetRange(int minBet, int maxBet);
    void setBetValue(int value);
    int getBetValue() const;
    void resetBetControls();

  signals:
    void foldClicked();
    void callClicked();
    void checkClicked();
    void betClicked(int amount);
    void raiseClicked(int amount);
    void allInClicked();

  private:
    void setupUi();
    void connectSignals();

    // Action controls
    QGroupBox* m_actionGroup = nullptr;
    QHBoxLayout* m_actionLayout = nullptr;
    QPushButton* m_foldButton = nullptr;
    QPushButton* m_callButton = nullptr;
    QPushButton* m_checkButton = nullptr;
    QPushButton* m_betButton = nullptr;
    QPushButton* m_raiseButton = nullptr;
    QPushButton* m_allInButton = nullptr;
    
    // Betting controls
    QGroupBox* m_bettingGroup = nullptr;
    QHBoxLayout* m_bettingLayout = nullptr;
    QSlider* m_betSlider = nullptr;
    QSpinBox* m_betSpinBox = nullptr;
    QLabel* m_betAmountLabel = nullptr;
    
    // State
    int m_currentCallAmount = 0;
    int m_minRaise = 0;
    int m_maxRaise = 0;
};

} // namespace pkt::ui::qtwidgets
