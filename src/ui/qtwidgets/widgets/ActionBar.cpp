#include "ActionBar.h"

namespace pkt::ui::qtwidgets
{

ActionBar::ActionBar(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    connectSignals();
}

void ActionBar::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // Create action buttons group
    m_actionGroup = new QGroupBox("Actions", this);
    m_actionGroup->setStyleSheet("QGroupBox {"
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
                                 "}");
    m_actionLayout = new QHBoxLayout(m_actionGroup);

    m_foldButton = new QPushButton("Fold", this);
    m_callButton = new QPushButton("Call", this);
    m_checkButton = new QPushButton("Check", this);
    m_betButton = new QPushButton("Bet", this);
    m_raiseButton = new QPushButton("Raise", this);
    m_allInButton = new QPushButton("All-In", this);

    // Unified, sober light style for all action buttons
    QString unifiedBtnStyle = "QPushButton {"
                              "  background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                              "    stop: 0 #ffffff, stop: 1 #f1f3f5);"
                              "  border: 2px solid #dee2e6;"
                              "  border-radius: 10px;"
                              "  padding: 12px 18px;"
                              "  color: #2c3e50;"
                              "  font-size: 14px;"
                              "  font-weight: 600;"
                              "  min-width: 88px;"
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

    // Create betting controls group
    m_bettingGroup = new QGroupBox("Betting Amount", this);
    m_bettingGroup->setStyleSheet("QGroupBox {"
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
                                  "}");
    m_bettingLayout = new QHBoxLayout(m_bettingGroup);

    m_betAmountLabel = new QLabel("Amount: $0", this);
    m_betSlider = new QSlider(Qt::Horizontal, this);
    m_betSpinBox = new QSpinBox(this);

    m_betAmountLabel->setStyleSheet("color: #495057; font-size: 14px;");

    m_betSlider->setMinimum(0);
    m_betSlider->setMaximum(1000);
    m_betSlider->setValue(0);
    m_betSlider->setMinimumWidth(200);

    m_betSpinBox->setMinimum(0);
    m_betSpinBox->setMaximum(10000);
    m_betSpinBox->setValue(0);
    m_betSpinBox->setMinimumWidth(80);

    m_bettingLayout->addWidget(m_betAmountLabel);
    m_bettingLayout->addWidget(m_betSlider);
    m_bettingLayout->addWidget(m_betSpinBox);

    m_bettingGroup->setMaximumWidth(350);
    m_bettingGroup->setEnabled(false);

    // Enhanced betting controls styling
    QString sliderStyle = "QSlider::groove:horizontal {"
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

    QString spinBoxStyle = "QSpinBox {"
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

    // Add groups to main layout
    mainLayout->addWidget(m_actionGroup, 3); // Action buttons take more space
    mainLayout->addWidget(m_bettingGroup, 1);

    // Disable all inputs initially
    enableInput(false);
}

void ActionBar::connectSignals()
{
    // Connect action buttons
    connect(m_foldButton, &QPushButton::clicked, this, &ActionBar::foldClicked);
    connect(m_callButton, &QPushButton::clicked, this, &ActionBar::callClicked);
    connect(m_checkButton, &QPushButton::clicked, this, &ActionBar::checkClicked);
    connect(m_allInButton, &QPushButton::clicked, this, &ActionBar::allInClicked);

    // Bet and raise buttons emit with amount
    connect(m_betButton, &QPushButton::clicked, this, [this]() {
        emit betClicked(m_betSpinBox->value());
    });
    connect(m_raiseButton, &QPushButton::clicked, this, [this]() {
        emit raiseClicked(m_betSpinBox->value());
    });

    // Sync slider and spinbox
    connect(m_betSlider, &QSlider::valueChanged, this, [this](int value) {
        m_betSpinBox->blockSignals(true);
        m_betSpinBox->setValue(value);
        m_betAmountLabel->setText(QString("Amount: $%1").arg(value));
        m_betSpinBox->blockSignals(false);
    });
    
    connect(m_betSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_betSlider->blockSignals(true);
        m_betSlider->setValue(value);
        m_betAmountLabel->setText(QString("Amount: $%1").arg(value));
        m_betSlider->blockSignals(false);
    });
}

void ActionBar::setAvailableActions(bool canFold, bool canCall, bool canCheck,
                                     bool canBet, bool canRaise, bool canAllIn)
{
    m_foldButton->setEnabled(canFold);
    m_callButton->setEnabled(canCall);
    m_checkButton->setEnabled(canCheck);
    m_betButton->setEnabled(canBet);
    m_raiseButton->setEnabled(canRaise);
    m_allInButton->setEnabled(canAllIn);

    // Enable betting controls if bet or raise is available
    m_bettingGroup->setEnabled(canBet || canRaise);
}

void ActionBar::updateCallAmount(int amount)
{
    m_currentCallAmount = amount;
    m_callButton->setText(QString("Call $%1").arg(amount));
}

void ActionBar::updateRaiseAmount(int minRaise, int maxRaise)
{
    m_minRaise = minRaise;
    m_maxRaise = maxRaise;
    
    setBetRange(minRaise, maxRaise);
    
    // Update button text
    m_raiseButton->setText(QString("Raise (min $%1)").arg(minRaise));
    m_betButton->setText(QString("Bet (min $%1)").arg(minRaise));
}

void ActionBar::enableInput(bool enabled)
{
    m_actionGroup->setEnabled(enabled);
    
    // Betting group enabled state is controlled by setAvailableActions
    if (!enabled)
    {
        m_bettingGroup->setEnabled(false);
    }
}

void ActionBar::setBetRange(int minBet, int maxBet)
{
    m_betSlider->setMinimum(minBet);
    m_betSlider->setMaximum(maxBet);
    m_betSpinBox->setMinimum(minBet);
    m_betSpinBox->setMaximum(maxBet);
    
    // Set to minimum by default
    if (minBet > 0)
    {
        setBetValue(minBet);
    }
}

void ActionBar::setBetValue(int value)
{
    m_betSlider->setValue(value);
    m_betSpinBox->setValue(value);
    m_betAmountLabel->setText(QString("Amount: $%1").arg(value));
}

int ActionBar::getBetValue() const
{
    return m_betSpinBox->value();
}

void ActionBar::resetBetControls()
{
    setBetValue(0);
    m_bettingGroup->setEnabled(false);
}

} // namespace pkt::ui::qtwidgets
