// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <core/engine/EngineDefs.h>
#include <core/engine/model/GameData.h>
#include <core/engine/model/GameState.h>
#include <core/engine/model/StartData.h>
#include "ui_GameTableWindow.h"

#include <core/session/Session.h>

#include <memory>
#include <string>

#include <QtCore>
#include <QtGui>

#define USER_WIDGETS_NUMBER 9

namespace pkt::core::player
{
class Player;
} // namespace pkt::core::player

namespace pkt::core
{
class Game;
class IBoard;
} // namespace pkt::core

class MyCardsPixmapLabel;
class MyCashLabel;
class MyNameLabel;

class settingsDialogImpl;
class StartWindow;

class GameTableStyleReader;
class CardDeckStyleReader;

enum SeatState
{
    SEAT_UNDEFINED,
    SEAT_ACTIVE,
    SEAT_AUTOFOLD,
    SEAT_STAYONTABLE,
    SEAT_CLEAR
};

class GameTableWindow : public QMainWindow, public Ui::gameTable
{
    Q_OBJECT

  public:
    GameTableWindow(const std::string& appDataDir, QMainWindow* parent = 0);

    ~GameTableWindow();

    pkt::core::Session* getSession();

    GameTableStyleReader* getGameTableStyle() const;

    void setStartWindow(StartWindow* s);

    void setSpeeds();

  signals:

    void signalShowClientDialog();

    void signalDealFlopCards0();
    void signalDealTurnCards0();
    void signalDealRiverCards0();

    void signalactivePlayerActionDone();

    void signalPreflopAnimation2();
    void signalFlopAnimation1();
    void signalFlopAnimation2();

    void signalTurnAnimation2();

    void signalRiverAnimation2();

  public slots:

    void gameInitializeGui(int speed);
    void hideHoleCards();

    void refreshSet();
    void refreshCash();
    void refreshAction(int playerId = -1, int playerAction = -1);
    void refreshPot();
    void refreshPlayersActiveInactiveStyles(int = -1, int = -1);

    void refreshPlayerName();
    void refreshTableDescriptiveLabels(int);
    void refreshDealerAndBlindsButtons();
    void refreshPlayerStatistics();

    void refreshHandsRanges();

    SeatState getCurrentSeatState(std::shared_ptr<pkt::core::player::Player>);

    void dealHoleCards();

    void provideMyActions(int mode = -1); // mode 0 == called from dealBettingRoundcards
    void doHumanAction();
    void disableMyButtons();

    void setGameSpeed(const int theValue);

    void pushButtonBetRaiseClicked(bool checked);
    void pushButtonBetRaiseHalfPotClicked(bool checked);
    void pushButtonBetRaiseTwoThirdPotClicked(bool checked);
    void pushButtonBetRaisePotClicked(bool checked);
    void pushButtonCallCheckClicked(bool checked);
    void pushButtonFoldClicked(bool checked);
    void pushButtonAllInClicked(bool checked);

    void myCallCheck();

    void myFold();
    void myCheck();
    int getCallAmount();
    int getBetRaisePushButtonValue();
    int getBetAmount();
    void myCall();
    void mySet();
    void myAllIn();

    void myActionDone();

    void dealBettingRoundCards(int);

    void dealFlopCards0();
    void dealFlopCards1();
    void dealFlopCards2();
    void dealFlopCards3();
    void dealFlopCards4();
    void dealFlopCards5();
    void dealFlopCards6();

    void dealTurnCards0();
    void dealTurnCards1();
    void dealTurnCards2();

    void dealRiverCards0();
    void dealRiverCards1();
    void dealRiverCards2();

    void activePlayerActionDone();

    void bettingRoundAnimation(int);

    void startPreflop();
    void preflopAnimation1Action();
    void preflopAnimation2();
    void preflopAnimation2Action();

    void flopAnimation1();
    void flopAnimation1Action();
    void flopAnimation2();
    void flopAnimation2Action();

    void turnAnimation1();
    void turnAnimation1Action();
    void turnAnimation2();
    void turnAnimation2Action();

    void riverAnimation1();
    void riverAnimation1Action();
    void riverAnimation2();
    void riverAnimation2Action();

    void postRiverAnimation1();
    void postRiverAnimation1Action();

    void postRiverRunAnimation1();
    void postRiverRunAnimation2();
    void postRiverRunAnimation3();
    void postRiverRunAnimation4();
    void postRiverRunAnimation5();
    void postRiverRunAnimation6();

    void showHoleCards(unsigned playerId, bool allIn = false);

    void blinkingStartButtonAnimationAction();

    void flipHoleCardsAllIn();
    void handSwitchRounds();

    void startNewHand();

    void stopAllGuiTimers();

    void nextBettingRoundInitializeGui();

    void breakButtonClicked();

    bool eventFilter(QObject* obj, QEvent* event);

    void switchLogWindow();
    void switchAwayWindow();
    void switchChanceWindow();
    void switchFullscreen();

    void GameModification();

    void updateHumanPlayerButtonsState(int mode = -1); // mode 0 == called from dealBettingRoundcards
    void uncheckMyButtons();
    void resetButtonsCheckStateMemory();
    void clearMyButtons();
    void myButtonsCheckable(bool state);

    void changeSpinBoxBetValue(int);
    void spinBoxBetValueChanged(int);

    void showMaximized();
    void closeGameTable();

    void refreshGameTableStyle();

    void closeMessageBoxes();
    void hide();
    void enableCallCheckPushButton();

  private:
    void initHoleCards();

    // Timer
    QTimer* potDistributeTimer;
    QTimer* timer;
    QTimer* dealFlopCards0Timer;
    QTimer* dealFlopCards1Timer;
    QTimer* dealFlopCards2Timer;
    QTimer* dealFlopCards3Timer;
    QTimer* dealFlopCards4Timer;
    QTimer* dealFlopCards5Timer;
    QTimer* dealFlopCards6Timer;
    QTimer* dealTurnCards0Timer;
    QTimer* dealTurnCards1Timer;
    QTimer* dealTurnCards2Timer;
    QTimer* dealRiverCards0Timer;
    QTimer* dealRiverCards1Timer;
    QTimer* dealRiverCards2Timer;

    QTimer* activePlayerActionDoneTimer;
    QTimer* preflopAnimation1Timer;
    QTimer* preflopAnimation2Timer;
    QTimer* flopAnimation1Timer;
    QTimer* flopAnimation2Timer;
    QTimer* turnAnimation1Timer;
    QTimer* turnAnimation2Timer;
    QTimer* riverAnimation1Timer;
    QTimer* riverAnimation2Timer;

    QTimer* postRiverAnimation1Timer;
    QTimer* postRiverRunAnimation1Timer;
    QTimer* postRiverRunAnimation2Timer;
    QTimer* postRiverRunAnimation2_flipHoleCards1Timer;
    QTimer* postRiverRunAnimation2_flipHoleCards2Timer;
    QTimer* postRiverRunAnimation3Timer;
    QTimer* postRiverRunAnimation5Timer;
    QTimer* postRiverRunAnimation6Timer;

    QTimer* blinkingStartButtonAnimationTimer;
    QTimer* enableCallCheckPushButtonTimer;

    QWidget* userWidgetsArray[USER_WIDGETS_NUMBER];
    QLabel* buttonLabelArray[MAX_NUMBER_OF_PLAYERS];
    MyCashLabel* cashLabelArray[MAX_NUMBER_OF_PLAYERS];
    QLabel* cashTopLabelArray[MAX_NUMBER_OF_PLAYERS];
    MySetLabel* setLabelArray[MAX_NUMBER_OF_PLAYERS];
    QLabel* actionLabelArray[MAX_NUMBER_OF_PLAYERS];
    MyNameLabel* playerNameLabelArray[MAX_NUMBER_OF_PLAYERS];
    QLabel* playerStarsArray[6][MAX_NUMBER_OF_PLAYERS];

    QGroupBox* groupBoxArray[MAX_NUMBER_OF_PLAYERS];
    MyCardsPixmapLabel* boardCardsArray[5];
    MyCardsPixmapLabel* holeCardsArray[MAX_NUMBER_OF_PLAYERS][2];

    QLabel* playerTipLabelArray[MAX_NUMBER_OF_PLAYERS];
    QPixmap flipside;

    // 	Dialogs
    StartWindow* myStartWindow;

    QString myAppDataDir;

    int distributePotAnimCounter;
    int playingMode;

    QString font2String;
    QString font1String;

    // Speed
    int guiGameSpeed;
    int gameSpeed;
    int dealCardsSpeed;
    int preDealCardsSpeed;
    int postDealCardsSpeed;
    int AllInDealCardsSpeed;
    int postRiverRunAnimationSpeed;
    int winnerBlinkSpeed;
    int newRoundSpeed;
    int nextPlayerSpeed1;
    int nextPlayerSpeed2;
    int nextPlayerSpeed3;
    int preflopNextPlayerSpeed;
    int nextOpponentSpeed;

    bool myActionIsBet;
    bool myActionIsRaise;
    bool pushButtonBetRaiseIsChecked;
    bool pushButtonBetRaiseHalfPotIsChecked;
    bool pushButtonBetRaiseTwoThirdPotIsChecked;
    bool pushButtonBetRaisePotIsChecked;
    bool pushButtonCallCheckIsChecked;
    bool pushButtonFoldIsChecked;
    bool pushButtonAllInIsChecked;
    bool myButtonsAreCheckable;
    bool breakAfterCurrentHand;
    bool currentGameOver;
    bool flipHoleCardsAllInAlreadyDone;
    bool betSliderChangedByInput;
    bool guestMode;

    // statistic testing
    int statisticArray[15];

    QSemaphore guiUpdateSemaphore;

    int keyUpDownChatCounter;
    int myLastPreActionBetValue;

    GameTableStyleReader* myGameTableStyle;
    CardDeckStyleReader* myCardDeckStyle;

    QString AllInString;
    QString RaiseString;
    QString BetString;
    QString CallString;
    QString CheckString;
    QString FoldString;
    QString PotString;
    QString TotalString;
    QString BetsString;
    QString GameString;
    QString HandString;
    QString PreflopString;
    QString FlopString;
    QString TurnString;
    QString RiverString;
};
