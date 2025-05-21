/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "GameTableWindow.h"
#include <core/session/Session.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

#include <ui/qtwidgets/gametable/GuiDisplayGameActions.h>
#include "myactionbutton.h"
#include "mycardspixmaplabel.h"
#include "mycashlabel.h"
#include "mychancelabel.h"
#include "mynamelabel.h"
#include "mysetlabel.h"

#include <core/engine/CardsValue.h>
#include <core/engine/Game.h>
#include <core/engine/model/ButtonState.h>

#include <core/engine/model/Ranges.h>

#include <core/interfaces/IBoard.h>
#include <core/interfaces/IHand.h>

#include <core/player/HumanPlayer.h>

#include <ui/qtwidgets/styles/carddeckstylereader.h>
#include <ui/qtwidgets/styles/gametablestylereader.h>

#include <core/interfaces/ILogger.h>

#include <QtCore/qregularexpression.h>

#include <cmath>

#define FORMATLEFT(X) "<p align='center'>(X)"
#define FORMATRIGHT(X) "(X)</p>"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

using namespace std;

GameTableWindow::GameTableWindow(const std::string& appDataDir, QMainWindow* parent)
    : QMainWindow(parent), gameSpeed(0), myActionIsBet(0), myActionIsRaise(0), pushButtonBetRaiseIsChecked(false),
      pushButtonBetRaiseHalfPotIsChecked(false), pushButtonBetRaiseTwoThirdPotIsChecked(false),
      pushButtonBetRaisePotIsChecked(false), pushButtonCallCheckIsChecked(false), pushButtonFoldIsChecked(false),
      pushButtonAllInIsChecked(false), myButtonsAreCheckable(false), breakAfterCurrentHand(false),
      currentGameOver(false), betSliderChangedByInput(false), guestMode(false), myLastPreActionBetValue(0)
{
    int i;

    // for statistic development
    for (i = 0; i < 15; i++)
    {
        statisticArray[i] = 0;
    }
    ////////////////////////////

    myAppDataDir = QString::fromStdString(appDataDir);

    setupUi(this);
    setGameSpeed(10);

    // 	Init game table style
    myGameTableStyle = new GameTableStyleReader(this, myAppDataDir);
    myGameTableStyle->readStyleFile("");

    // 	Init card deck style
    myCardDeckStyle = new CardDeckStyleReader(this);
    myCardDeckStyle->readStyleFile("", appDataDir);

    // Player0 pixmapCardsLabel needs Myw
    pixmapLabel_card0b->setGameTableWindow(this);
    pixmapLabel_card0a->setGameTableWindow(this);

    label_chance->setStyle(myGameTableStyle);

    flipside = QPixmap::fromImage(QImage(myCardDeckStyle->getCurrentDir() + "flipside.png"));

    flipHolecardsAllInAlreadyDone = false;

    // userWidgetsArray init
    userWidgetsArray[0] = pushButton_BetRaise;
    userWidgetsArray[1] = pushButton_BetRaiseHalfPot;
    userWidgetsArray[2] = pushButton_BetRaiseTwoThirdPot;
    userWidgetsArray[3] = pushButton_BetRaisePot;
    userWidgetsArray[4] = pushButton_CallCheck;
    userWidgetsArray[5] = pushButton_Fold;
    userWidgetsArray[6] = spinBox_betValue;
    userWidgetsArray[7] = horizontalSlider_bet;
    userWidgetsArray[8] = pushButton_AllIn;

    // hide userWidgets
    for (i = 0; i < USER_WIDGETS_NUMBER; i++)
    {
        userWidgetsArray[i]->hide();
    }

    // Timer Objekt erstellen
    dealFlopCards0Timer = new QTimer(this);
    dealFlopCards1Timer = new QTimer(this);
    dealFlopCards2Timer = new QTimer(this);
    dealFlopCards3Timer = new QTimer(this);
    dealFlopCards4Timer = new QTimer(this);
    dealFlopCards5Timer = new QTimer(this);
    dealFlopCards6Timer = new QTimer(this);
    dealTurnCards0Timer = new QTimer(this);
    dealTurnCards1Timer = new QTimer(this);
    dealTurnCards2Timer = new QTimer(this);
    dealRiverCards0Timer = new QTimer(this);
    dealRiverCards1Timer = new QTimer(this);
    dealRiverCards2Timer = new QTimer(this);

    nextPlayerAnimationTimer = new QTimer(this);
    preflopAnimation1Timer = new QTimer(this);
    preflopAnimation2Timer = new QTimer(this);
    flopAnimation1Timer = new QTimer(this);
    flopAnimation2Timer = new QTimer(this);
    turnAnimation1Timer = new QTimer(this);
    turnAnimation2Timer = new QTimer(this);
    riverAnimation1Timer = new QTimer(this);
    riverAnimation2Timer = new QTimer(this);

    postRiverAnimation1Timer = new QTimer(this);
    postRiverRunAnimation1Timer = new QTimer(this);
    postRiverRunAnimation2Timer = new QTimer(this);
    postRiverRunAnimation2_flipHoleCards1Timer = new QTimer(this);
    postRiverRunAnimation2_flipHoleCards2Timer = new QTimer(this);
    postRiverRunAnimation3Timer = new QTimer(this);
    postRiverRunAnimation5Timer = new QTimer(this);
    potDistributeTimer = new QTimer(this);
    postRiverRunAnimation6Timer = new QTimer(this);

    blinkingStartButtonAnimationTimer = new QTimer(this);
    //	voteOnKickTimeoutTimer = new QTimer(this);
    enableCallCheckPushButtonTimer = new QTimer(this);

    dealFlopCards0Timer->setSingleShot(true);
    dealFlopCards1Timer->setSingleShot(true);
    dealFlopCards2Timer->setSingleShot(true);
    dealFlopCards3Timer->setSingleShot(true);
    dealFlopCards4Timer->setSingleShot(true);
    dealFlopCards5Timer->setSingleShot(true);
    dealFlopCards6Timer->setSingleShot(true);
    dealTurnCards0Timer->setSingleShot(true);
    dealTurnCards1Timer->setSingleShot(true);
    dealTurnCards2Timer->setSingleShot(true);
    dealRiverCards0Timer->setSingleShot(true);
    dealRiverCards1Timer->setSingleShot(true);
    dealRiverCards2Timer->setSingleShot(true);

    nextPlayerAnimationTimer->setSingleShot(true);
    preflopAnimation1Timer->setSingleShot(true);
    preflopAnimation2Timer->setSingleShot(true);
    flopAnimation1Timer->setSingleShot(true);
    flopAnimation2Timer->setSingleShot(true);
    turnAnimation1Timer->setSingleShot(true);
    turnAnimation2Timer->setSingleShot(true);
    riverAnimation1Timer->setSingleShot(true);
    riverAnimation2Timer->setSingleShot(true);

    postRiverAnimation1Timer->setSingleShot(true);
    postRiverRunAnimation1Timer->setSingleShot(true);
    postRiverRunAnimation2Timer->setSingleShot(true);
    postRiverRunAnimation3Timer->setSingleShot(true);
    postRiverRunAnimation5Timer->setSingleShot(true);
    postRiverRunAnimation6Timer->setSingleShot(true);

    enableCallCheckPushButtonTimer->setSingleShot(true);

    // buttonLabelArray init
    buttonLabelArray[0] = textLabel_Button0;
    buttonLabelArray[1] = textLabel_Button1;
    buttonLabelArray[2] = textLabel_Button2;
    buttonLabelArray[3] = textLabel_Button3;
    buttonLabelArray[4] = textLabel_Button4;
    buttonLabelArray[5] = textLabel_Button5;
    buttonLabelArray[6] = textLabel_Button6;
    buttonLabelArray[7] = textLabel_Button7;
    buttonLabelArray[8] = textLabel_Button8;
    buttonLabelArray[9] = textLabel_Button9;

    // cashLabelArray init
    cashLabelArray[0] = textLabel_Cash0;
    cashLabelArray[1] = textLabel_Cash1;
    cashLabelArray[2] = textLabel_Cash2;
    cashLabelArray[3] = textLabel_Cash3;
    cashLabelArray[4] = textLabel_Cash4;
    cashLabelArray[5] = textLabel_Cash5;
    cashLabelArray[6] = textLabel_Cash6;
    cashLabelArray[7] = textLabel_Cash7;
    cashLabelArray[8] = textLabel_Cash8;
    cashLabelArray[9] = textLabel_Cash9;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        cashLabelArray[i]->setGameTableWindow(this);
    }

    playerTipLabelArray[0] = label_playerTip0;
    playerTipLabelArray[1] = label_playerTip1;
    playerTipLabelArray[2] = label_playerTip2;
    playerTipLabelArray[3] = label_playerTip3;
    playerTipLabelArray[4] = label_playerTip4;
    playerTipLabelArray[5] = label_playerTip5;
    playerTipLabelArray[6] = label_playerTip6;
    playerTipLabelArray[7] = label_playerTip7;
    playerTipLabelArray[8] = label_playerTip8;
    playerTipLabelArray[9] = label_playerTip9;

    // playerNameLabelArray init
    playerNameLabelArray[0] = label_PlayerName0;
    playerNameLabelArray[1] = label_PlayerName1;
    playerNameLabelArray[2] = label_PlayerName2;
    playerNameLabelArray[3] = label_PlayerName3;
    playerNameLabelArray[4] = label_PlayerName4;
    playerNameLabelArray[5] = label_PlayerName5;
    playerNameLabelArray[6] = label_PlayerName6;
    playerNameLabelArray[7] = label_PlayerName7;
    playerNameLabelArray[8] = label_PlayerName8;
    playerNameLabelArray[9] = label_PlayerName9;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        playerNameLabelArray[i]->setGameTableWindow(this);
    }

    // setLabelArray init
    setLabelArray[0] = textLabel_Set0;
    setLabelArray[1] = textLabel_Set1;
    setLabelArray[2] = textLabel_Set2;
    setLabelArray[3] = textLabel_Set3;
    setLabelArray[4] = textLabel_Set4;
    setLabelArray[5] = textLabel_Set5;
    setLabelArray[6] = textLabel_Set6;
    setLabelArray[7] = textLabel_Set7;
    setLabelArray[8] = textLabel_Set8;
    setLabelArray[9] = textLabel_Set9;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        setLabelArray[i]->setGameTableWindow(this);
    }

    // statusLabelArray init
    actionLabelArray[0] = textLabel_Status0;
    actionLabelArray[1] = textLabel_Status1;
    actionLabelArray[2] = textLabel_Status2;
    actionLabelArray[3] = textLabel_Status3;
    actionLabelArray[4] = textLabel_Status4;
    actionLabelArray[5] = textLabel_Status5;
    actionLabelArray[6] = textLabel_Status6;
    actionLabelArray[7] = textLabel_Status7;
    actionLabelArray[8] = textLabel_Status8;
    actionLabelArray[9] = textLabel_Status9;

    textLabel_Status0->setGameTableWindow(this);

    // GroupBoxArray init
    groupBoxArray[0] = groupBox0;
    groupBoxArray[1] = groupBox1;
    groupBoxArray[2] = groupBox2;
    groupBoxArray[3] = groupBox3;
    groupBoxArray[4] = groupBox4;
    groupBoxArray[5] = groupBox5;
    groupBoxArray[6] = groupBox6;
    groupBoxArray[7] = groupBox7;
    groupBoxArray[8] = groupBox8;
    groupBoxArray[9] = groupBox9;

    // boardCardsArray init
    boardCardsArray[0] = pixmapLabel_cardBoard0;
    boardCardsArray[1] = pixmapLabel_cardBoard1;
    boardCardsArray[2] = pixmapLabel_cardBoard2;
    boardCardsArray[3] = pixmapLabel_cardBoard3;
    boardCardsArray[4] = pixmapLabel_cardBoard4;

    initHoleCards();

    // style Game Table
    refreshGameTableStyle();

    // raise actionLable above just inserted mypixmaplabel
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        actionLabelArray[i]->raise();
    }

    // raise board cards
    for (i = 0; i < 5; i++)
    {
        boardCardsArray[i]->raise();
    }

    // fix for away string bug in righttabwidget on windows
    tabWidget_Right->setTabText(0, " " + tabWidget_Right->tabText(0) + " ");
    tabWidget_Right->setTabText(1, " " + tabWidget_Right->tabText(1) + " ");
    tabWidget_Right->setTabText(2, " " + tabWidget_Right->tabText(2) + " ");

    // resize stop-button depending on translation
    QFontMetrics tempMetrics = this->fontMetrics();
    int width = tempMetrics.horizontalAdvance((tr("Show cards")));

    // Clear Focus

    pushButton_break->setMinimumSize(width + 10, 20);
    groupBox_RightToolBox->clearFocus();

    // set Focus to gametable
    this->setFocus();

    // windowicon
    //  	QString windowIconString();
    this->setWindowIcon(QIcon(myAppDataDir + "gfx/gui/misc/windowicon.png"));

    this->installEventFilter(this);

    this->setWindowTitle(QString(tr("PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));

    connect(dealFlopCards0Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards1()));
    connect(dealFlopCards1Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards2()));
    connect(dealFlopCards2Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards3()));
    connect(dealFlopCards3Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards4()));
    connect(dealFlopCards4Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards5()));
    connect(dealFlopCards5Timer, SIGNAL(timeout()), this, SLOT(dealFlopCards6()));
    connect(dealFlopCards6Timer, SIGNAL(timeout()), this, SLOT(handSwitchRounds()));
    connect(dealTurnCards0Timer, SIGNAL(timeout()), this, SLOT(dealTurnCards1()));
    connect(dealTurnCards1Timer, SIGNAL(timeout()), this, SLOT(dealTurnCards2()));
    connect(dealTurnCards2Timer, SIGNAL(timeout()), this, SLOT(handSwitchRounds()));
    connect(dealRiverCards0Timer, SIGNAL(timeout()), this, SLOT(dealRiverCards1()));
    connect(dealRiverCards1Timer, SIGNAL(timeout()), this, SLOT(dealRiverCards2()));
    connect(dealRiverCards2Timer, SIGNAL(timeout()), this, SLOT(handSwitchRounds()));

    connect(nextPlayerAnimationTimer, SIGNAL(timeout()), this, SLOT(handSwitchRounds()));
    connect(preflopAnimation1Timer, SIGNAL(timeout()), this, SLOT(preflopAnimation1Action()));
    connect(preflopAnimation2Timer, SIGNAL(timeout()), this, SLOT(preflopAnimation2Action()));
    connect(flopAnimation1Timer, SIGNAL(timeout()), this, SLOT(flopAnimation1Action()));
    connect(flopAnimation2Timer, SIGNAL(timeout()), this, SLOT(flopAnimation2Action()));
    connect(turnAnimation1Timer, SIGNAL(timeout()), this, SLOT(turnAnimation1Action()));
    connect(turnAnimation2Timer, SIGNAL(timeout()), this, SLOT(turnAnimation2Action()));
    connect(riverAnimation1Timer, SIGNAL(timeout()), this, SLOT(riverAnimation1Action()));
    connect(riverAnimation2Timer, SIGNAL(timeout()), this, SLOT(riverAnimation2Action()));

    connect(postRiverAnimation1Timer, SIGNAL(timeout()), this, SLOT(postRiverAnimation1Action()));
    connect(postRiverRunAnimation1Timer, SIGNAL(timeout()), this, SLOT(postRiverRunAnimation2()));
    connect(postRiverRunAnimation2Timer, SIGNAL(timeout()), this, SLOT(postRiverRunAnimation3()));
    connect(postRiverRunAnimation3Timer, SIGNAL(timeout()), this, SLOT(postRiverRunAnimation4()));
    connect(potDistributeTimer, SIGNAL(timeout()), this, SLOT(postRiverRunAnimation5()));
    connect(postRiverRunAnimation5Timer, SIGNAL(timeout()), this, SLOT(postRiverRunAnimation6()));
    connect(postRiverRunAnimation6Timer, SIGNAL(timeout()), this, SLOT(startNewHand()));

    connect(blinkingStartButtonAnimationTimer, SIGNAL(timeout()), this, SLOT(blinkingStartButtonAnimationAction()));
    connect(enableCallCheckPushButtonTimer, SIGNAL(timeout()), this, SLOT(enableCallCheckPushButton()));

    connect(actionClose, SIGNAL(triggered()), this, SLOT(closeGameTable()));

    connect(actionFullScreen, SIGNAL(triggered()), this, SLOT(switchFullscreen()));

    connect(actionShowHideLog, SIGNAL(triggered()), this, SLOT(switchLogWindow()));
    connect(actionShowHideChance, SIGNAL(triggered()), this, SLOT(switchChanceWindow()));

    connect(pushButton_BetRaise, SIGNAL(clicked(bool)), this, SLOT(pushButtonBetRaiseClicked(bool)));
    connect(pushButton_BetRaiseHalfPot, SIGNAL(clicked(bool)), this, SLOT(pushButtonBetRaiseHalfPotClicked(bool)));
    connect(pushButton_BetRaiseTwoThirdPot, SIGNAL(clicked(bool)), this,
            SLOT(pushButtonBetRaiseTwoThirdPotClicked(bool)));
    connect(pushButton_BetRaisePot, SIGNAL(clicked(bool)), this, SLOT(pushButtonBetRaisePotClicked(bool)));

    connect(pushButton_Fold, SIGNAL(clicked(bool)), this, SLOT(pushButtonFoldClicked(bool)));
    connect(pushButton_CallCheck, SIGNAL(clicked(bool)), this, SLOT(pushButtonCallCheckClicked(bool)));
    connect(pushButton_AllIn, SIGNAL(clicked(bool)), this, SLOT(pushButtonAllInClicked(bool)));
    connect(horizontalSlider_bet, SIGNAL(valueChanged(int)), this, SLOT(changeSpinBoxBetValue(int)));
    connect(spinBox_betValue, SIGNAL(valueChanged(int)), this, SLOT(spinBoxBetValueChanged(int)));

    connect(pushButton_break, SIGNAL(clicked()), this, SLOT(breakButtonClicked()));

    connect(radioButton_manualAction, SIGNAL(clicked()), this, SLOT(changePlayingMode()));
    connect(radioButton_autoCheckFold, SIGNAL(clicked()), this, SLOT(changePlayingMode()));
    connect(radioButton_autoCheckCallAny, SIGNAL(clicked()), this, SLOT(changePlayingMode()));

    connect(this, SIGNAL(signalInitGui(int)), this, SLOT(initGui(int)));
    // connect(this, SIGNAL(signalRefreshSet()), this, SLOT(refreshSet()));
    // connect(this, SIGNAL(signalRefreshCash()), this, SLOT(refreshCash()));
    // connect(this, SIGNAL(signalRefreshAction(int, int)), this, SLOT(refreshAction(int, int)));
    // connect(this, SIGNAL(signalRefreshChangePlayer()), this, SLOT(refreshChangePlayer()));
    // connect(this, SIGNAL(signalRefreshPot()), this, SLOT(refreshPot()));
    connect(this, SIGNAL(signalrefreshPlayersActiveInactiveStyles(int, int)), this,
            SLOT(refreshPlayersActiveInactiveStyles(int, int)));
    // connect(this, SIGNAL(signalRefreshAll()), this, SLOT(refreshAll()));
    // connect(this, SIGNAL(signalRefreshPlayerName()), this, SLOT(refreshPlayerName()));
    connect(this, SIGNAL(signalRefreshButton()), this, SLOT(refreshButton()));
    connect(this, SIGNAL(signalRefreshGameLabels(int)), this, SLOT(refreshGameLabels(int)));
    connect(this, SIGNAL(signalGuiUpdateDone()), this, SLOT(guiUpdateDone()));
    connect(this, SIGNAL(signalMeInAction()), this, SLOT(meInAction()));
    connect(this, SIGNAL(signalDisableMyButtons()), this, SLOT(disableMyButtons()));
    connect(this, SIGNAL(signalUpdateMyButtonsState()), this, SLOT(updateMyButtonsState()));
    // connect(this, SIGNAL(signalDealBettingRoundCards(int)), this, SLOT(dealBettingRoundCards(int)));
    // connect(this, SIGNAL(signalDealHoleCards()), this, SLOT(dealHoleCards()));
    connect(this, SIGNAL(signalDealFlopCards0()), this, SLOT(dealFlopCards0()));
    connect(this, SIGNAL(signalDealTurnCards0()), this, SLOT(dealTurnCards0()));
    connect(this, SIGNAL(signalDealRiverCards0()), this, SLOT(dealRiverCards0()));
    connect(this, SIGNAL(signalNextPlayerAnimation()), this, SLOT(nextPlayerAnimation()));
    connect(this, SIGNAL(signalBettingRoundAnimation2(int)), this, SLOT(bettingRoundAnimation(int)));
    connect(this, SIGNAL(signalPreflopAnimation1()), this, SLOT(preflopAnimation1()));
    connect(this, SIGNAL(signalPreflopAnimation2()), this, SLOT(preflopAnimation2()));
    connect(this, SIGNAL(signalFlopAnimation1()), this, SLOT(flopAnimation1()));
    connect(this, SIGNAL(signalFlopAnimation2()), this, SLOT(flopAnimation2()));
    connect(this, SIGNAL(signalTurnAnimation1()), this, SLOT(turnAnimation1()));
    connect(this, SIGNAL(signalTurnAnimation2()), this, SLOT(turnAnimation2()));
    connect(this, SIGNAL(signalRiverAnimation1()), this, SLOT(riverAnimation1()));
    connect(this, SIGNAL(signalRiverAnimation2()), this, SLOT(riverAnimation2()));
    connect(this, SIGNAL(signalPostRiverAnimation1()), this, SLOT(postRiverAnimation1()));
    connect(this, SIGNAL(signalPostRiverRunAnimation1()), this, SLOT(postRiverRunAnimation1()));
    connect(this, SIGNAL(signalPostRiverShowCards(unsigned)), this, SLOT(showHoleCards(unsigned)));
    connect(this, SIGNAL(signalFlipHolecardsAllIn()), this, SLOT(flipHolecardsAllIn()));
    connect(this, SIGNAL(signalNextRoundCleanGui()), this, SLOT(nextRoundCleanGui()));
}

GameTableWindow::~GameTableWindow()
{
}
void GameTableWindow::initHoleCards()
{

    // holeCardsArray int
    holeCardsArray[0][0] = pixmapLabel_card0a;
    holeCardsArray[0][1] = pixmapLabel_card0b;
    holeCardsArray[1][0] = pixmapLabel_card1a;
    holeCardsArray[1][1] = pixmapLabel_card1b;
    holeCardsArray[2][0] = pixmapLabel_card2a;
    holeCardsArray[2][1] = pixmapLabel_card2b;
    holeCardsArray[3][0] = pixmapLabel_card3a;
    holeCardsArray[3][1] = pixmapLabel_card3b;
    holeCardsArray[4][0] = pixmapLabel_card4a;
    holeCardsArray[4][1] = pixmapLabel_card4b;
    holeCardsArray[5][0] = pixmapLabel_card5a;
    holeCardsArray[5][1] = pixmapLabel_card5b;
    holeCardsArray[6][0] = pixmapLabel_card6a;
    holeCardsArray[6][1] = pixmapLabel_card6b;
    holeCardsArray[7][0] = pixmapLabel_card7a;
    holeCardsArray[7][1] = pixmapLabel_card7b;
    holeCardsArray[8][0] = pixmapLabel_card8a;
    holeCardsArray[8][1] = pixmapLabel_card8b;
    holeCardsArray[9][0] = pixmapLabel_card9a;
    holeCardsArray[9][1] = pixmapLabel_card9b;
}
void GameTableWindow::hideHoleCards()
{

    QPixmap onePix = QPixmap::fromImage(QImage(myAppDataDir + "gfx/gui/misc/1px.png"));

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        actionLabelArray[i]->setPixmap(onePix);
        holeCardsArray[i][0]->setPixmap(onePix, false);
        holeCardsArray[i][1]->setPixmap(onePix, false);
        buttonLabelArray[i]->setText("");
        buttonLabelArray[i]->setPixmap(onePix);
        cashLabelArray[i]->setText("");
        playerTipLabelArray[i]->setText("");
        playerNameLabelArray[i]->setText("");
        setLabelArray[i]->setText("");
    }
}

void GameTableWindow::initGui(int speed)
{
    // kill running Singleshots!!!
    stopTimer();

    label_Pot->setText(PotString);

    label_handNumber->setText(HandString + ":");
    label_gameNumber->setText(GameString + ":");

    label_Total->setText(TotalString + ":");
    label_Sets->setText(BetsString + ":");
    groupBox_RightToolBox->setDisabled(false);

    // show human player buttons
    for (int i = 0; i < USER_WIDGETS_NUMBER; i++)
    {
        userWidgetsArray[i]->show();
    }
}

Session* GameTableWindow::getSession()
{
    assert(myStartWindow->getSession());
    return myStartWindow->getSession();
}

// refresh-Funktionen
void GameTableWindow::refreshSet()
{

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        if ((*it_c)->getSet() == 0)
            setLabelArray[(*it_c)->getID()]->setText("");
        else
            setLabelArray[(*it_c)->getID()]->setText("$" + QString("%L1").arg((*it_c)->getSet()));
    }
}

void GameTableWindow::refreshButton()
{

    QPixmap dealerButton = QPixmap::fromImage(QImage(myGameTableStyle->getDealerPuck()));
    QPixmap smallblindButton = QPixmap::fromImage(QImage(myGameTableStyle->getSmallBlindPuck()));
    QPixmap bigblindButton = QPixmap::fromImage(QImage(myGameTableStyle->getBigBlindPuck()));
    QPixmap onePix = QPixmap::fromImage(QImage(myAppDataDir + "gfx/gui/misc/1px.png"));

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();
    PlayerList activePlayerList = currentGame->getActivePlayerList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        if ((*it_c)->getActiveStatus())
        {
            if (activePlayerList->size() > 2)
            {
                switch ((*it_c)->getButton())
                {

                case 1:
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(dealerButton);
                    break;
                case 2:
                {
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(smallblindButton);
                }
                break;
                case 3:
                {
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(bigblindButton);
                }
                break;
                default:
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(onePix);
                }
            }
            else
            {
                switch ((*it_c)->getButton())
                {

                case 2:
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(dealerButton);
                    break;
                case 3:
                {
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(bigblindButton);
                }
                break;
                default:
                    buttonLabelArray[(*it_c)->getID()]->setPixmap(onePix);
                }
            }
        }
        else
        {
            buttonLabelArray[(*it_c)->getID()]->setPixmap(onePix);
        }
    }
}

void GameTableWindow::refreshPlayerName()
{

    if (myStartWindow->getSession()->getCurrentGame())
    {

        std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();
        PlayerListConstIterator it_c;
        PlayerList seatsList = currentGame->getSeatsList();
        for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
        {

            // collect needed infos
            //  bool guest = myStartWindow->getSession()->getClientPlayerInfo((*it_c)->getID()).isGuest;
            bool guest = false;

            bool computerPlayer = false;
            if ((*it_c)->getType() == PLAYER_TYPE_COMPUTER)
            {
                computerPlayer = true;
            }
            QString nick = QString::fromUtf8((*it_c)->getName().c_str());

            // check SeatStates and refresh
            switch (getCurrentSeatState((*it_c)))
            {

            case SEAT_ACTIVE:
            {
                playerNameLabelArray[(*it_c)->getID()]->setText(nick, false, guest, computerPlayer);
            }
            break;
            case SEAT_AUTOFOLD:
            {
                playerNameLabelArray[(*it_c)->getID()]->setText(nick, true, guest, computerPlayer);
            }
            break;
            case SEAT_STAYONTABLE:
            {
                playerNameLabelArray[(*it_c)->getID()]->setText(nick, true, guest, computerPlayer);
            }
            break;
            case SEAT_CLEAR:
            {
                playerNameLabelArray[(*it_c)->getID()]->setText("");
            }
            break;
            default:
            {
                playerNameLabelArray[(*it_c)->getID()]->setText("");
            }
            }
        }
    }
}

void GameTableWindow::refreshAction(int playerID, int playerAction)
{

    QPixmap onePix = QPixmap::fromImage(QImage(myAppDataDir + "gfx/gui/misc/1px.png"));
    QPixmap action;

    QStringList actionArray;
    actionArray << "" << "fold" << "check" << "call" << "bet" << "raise" << "allin";

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    if (playerID == -1 || playerAction == -1)
    {

        PlayerListConstIterator it_c;
        PlayerList seatsList = currentGame->getSeatsList();
        for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
        {

            // if no action --> clear Pixmap
            if ((*it_c)->getAction() == 0)
            {
                actionLabelArray[(*it_c)->getID()]->setPixmap(onePix);
            }
            else
            {
                // paint action pixmap
                actionLabelArray[(*it_c)->getID()]->setPixmap(
                    QPixmap::fromImage(QImage(myGameTableStyle->getActionPic((*it_c)->getAction()))));
            }

            if ((*it_c)->getAction() == PLAYER_ACTION_FOLD)
            {

                if ((*it_c)->getID() != 0)
                {
                    holeCardsArray[(*it_c)->getID()][0]->setPixmap(onePix, false);
                    holeCardsArray[(*it_c)->getID()][1]->setPixmap(onePix, false);
                }
            }
        }
    }
    else
    {
        // if no action --> clear Pixmap
        if (playerAction == 0)
        {
            actionLabelArray[playerID]->setPixmap(onePix);
        }
        else
        {

            // 		paint action pixmap and raise
            actionLabelArray[playerID]->setPixmap(
                QPixmap::fromImage(QImage(myGameTableStyle->getActionPic(playerAction))));
        }

        if (playerAction == 1)
        { // FOLD

            if (playerID == 0)
            {
                holeCardsArray[0][0]->startFadeOut(10);
                holeCardsArray[0][1]->startFadeOut(10);
            }
            else
            {
                holeCardsArray[playerID][0]->setPixmap(onePix, false);
                holeCardsArray[playerID][1]->setPixmap(onePix, false);
            }
        }
    }
}
void GameTableWindow::refreshPlayerStatistics()
{
    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();
    const int nbPlayers = currentGame->getCurrentHand()->getActivePlayerList()->size();

    const string style = "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
                         "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Ubuntu';  font-size:8pt; "
                         "font-weight:400;\">"; // color:#ff0000;\">";

    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();

    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {

        playerTipLabelArray[(*it_c)->getID()]->raise();

        if (!(*it_c)->getActiveStatus())
        {
            playerTipLabelArray[(*it_c)->getID()]->setText("");
            playerTipLabelArray[(*it_c)->getID()]->setToolTip("");
            continue;
        }

        const PlayerStatistics& stats = (*it_c)->getStatistics(nbPlayers);
        const PreflopStatistics& preflop = stats.getPreflopStatistics();
        const FlopStatistics& flop = stats.getFlopStatistics();
        const TurnStatistics& turn = stats.getTurnStatistics();
        const RiverStatistics& river = stats.getRiverStatistics();

        stringstream displayTooltip;
        displayTooltip.precision(1);

        displayTooltip << fixed << style << (*it_c)->getName() << tr(" - stats for a ").toStdString() << nbPlayers
                       << " " << tr("players table").toStdString() << "</span></p>" << style << "<br><b>"
                       << tr("Preflop stats").toStdString() << "</b>"
                       << " " << tr("(hands played:").toStdString() << " " << preflop.m_hands << ")</span></p>" << style
                       << "VP$IP: " << preflop.getVoluntaryPutMoneyInPot() << "%, " << tr("raise").toStdString() << ": "
                       << preflop.getPreflopRaise() << "%</span></p>" << style << "3-Bet: " << preflop.getPreflop3Bet()
                       << "%, " << tr("Call to 3Bet:").toStdString() << " " << preflop.getPreflopCall3BetsFrequency()
                       << "%, 4-Bet: " << preflop.getPreflop4Bet() << "%</span></p>"

                       << style << "<br><b>" << tr("Flop stats").toStdString() << "</b> "
                       << tr("(hands played:").toStdString() << " " << flop.m_hands << ")</span></p>" << style
                       << tr("Agression Freq:").toStdString() << " " << flop.getAgressionFrequency()
                       << "%, AF: " << flop.getAgressionFactor()
                       << "<br>Continuation Bet: " << flop.getContinuationBetFrequency() << "%</span></p>"

                       << style << "<br><b>" << tr("Turn stats").toStdString() << "</b> "
                       << tr("(hands played:").toStdString() << " " << turn.m_hands << ")</span></p>" << style
                       << tr("Agression Freq:").toStdString() << " " << turn.getAgressionFrequency()
                       << "%, AF: " << turn.getAgressionFactor() << "</span></p>"

                       << style << "<br><b>" << tr("River stats").toStdString() << "</b> "
                       << tr("(hands played:").toStdString() << " " << river.m_hands << ")</span></p>" << style
                       << tr("Agression Freq:").toStdString() << " " << river.getAgressionFrequency()
                       << "%, AF: " << river.getAgressionFactor() << "<br>" << tr("Went to showdown:").toStdString()
                       << " " << stats.getWentToShowDown() << "%</span></p>";

        playerTipLabelArray[(*it_c)->getID()]->setToolTip(displayTooltip.str().c_str());
    }
}

void GameTableWindow::refreshCash()
{

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    bool transparent = true;
    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {

        // check SeatStates and refresh
        switch (getCurrentSeatState((*it_c)))
        {

        case SEAT_ACTIVE:
        {
            //			qDebug() << (*it_c)->getID() << "CASH ACTIVE";
            cashLabelArray[(*it_c)->getID()]->setText("$" + QString("%L1").arg((*it_c)->getCash()));
        }
        break;
        case SEAT_AUTOFOLD:
        {
            //			qDebug() << (*it_c)->getID() << "CASH AUTOFOLD"; //TODO transparent
            cashLabelArray[(*it_c)->getID()]->setText("$" + QString("%L1").arg((*it_c)->getCash()), transparent);
        }
        break;
        case SEAT_STAYONTABLE:
        {
            cashLabelArray[(*it_c)->getID()]->setText("");
        }
        break;
        case SEAT_CLEAR:
        {
            cashLabelArray[(*it_c)->getID()]->setText("");
        }
        break;
        default:
        {
            cashLabelArray[(*it_c)->getID()]->setText("");
        }
        }
    }
}

void GameTableWindow::refreshPlayersActiveInactiveStyles(int playerID, int status)
{

    int j;

    if (playerID == -1 || status == -1)
    {

        std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();
        PlayerListConstIterator it_c;
        PlayerList seatsList = currentGame->getSeatsList();
        for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
        {

            if ((*it_c)->getTurn())
            {
                myGameTableStyle->setPlayerSeatActiveStyle(groupBoxArray[(*it_c)->getID()]);
            }
            else
            {
                if ((*it_c)->getActiveStatus())
                {
                    if ((*it_c)->getID() == 0)
                    {
                        // show buttons
                        for (j = 0; j < USER_WIDGETS_NUMBER; j++)
                        {
                            userWidgetsArray[j]->show();
                        }
                    }
                    myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[(*it_c)->getID()]);
                }
                else
                {
                    if ((*it_c)->getID() == 0)
                    {
                        // hide buttons
                        for (j = 0; j < USER_WIDGETS_NUMBER; j++)
                        {
                            userWidgetsArray[j]->hide();
                        }
                        // disable anti-peek front after player is out
                        holeCardsArray[0][0]->signalFastFlipCards(false);
                        holeCardsArray[0][1]->signalFastFlipCards(false);
                    }
                    myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[(*it_c)->getID()]);
                }
            }
        }
    }
    else
    {
        switch (status)
        {

            // inactive
        case 0:
        {
            if (!playerID)
            {
                // hide buttons
                for (j = 0; j < USER_WIDGETS_NUMBER; j++)
                {
                    userWidgetsArray[j]->hide();
                }
                // disable anti-peek front after player is out
                holeCardsArray[0][0]->signalFastFlipCards(false);
                holeCardsArray[0][1]->signalFastFlipCards(false);
            }
            myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[playerID]);
        }
        break;
        // active but fold
        case 1:
        {
            if (!playerID)
            {
                // show buttons
                for (j = 0; j < USER_WIDGETS_NUMBER; j++)
                {
                    userWidgetsArray[j]->show();
                }
            }
            myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[playerID]);
        }
        break;
        // active in action
        case 2:
        {
            myGameTableStyle->setPlayerSeatActiveStyle(groupBoxArray[playerID]);
        }
        break;
        // active not in action
        case 3:
        {
            myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[playerID]);
        }
        break;
        default:
        {
        }
        }
    }
}

void GameTableWindow::refreshGameLabels(int gameState)
{

    switch (gameState)
    {
    case 0:
    {
        textLabel_handLabel->setText(PreflopString);
    }
    break;
    case 1:
    {
        textLabel_handLabel->setText(FlopString);
    }
    break;
    case 2:
    {
        textLabel_handLabel->setText(TurnString);
    }
    break;
    case 3:
    {
        textLabel_handLabel->setText(RiverString);
    }
    break;
    case 4:
    {
        textLabel_handLabel->setText("");
    }
    break;
    default:
    {
        textLabel_handLabel->setText("!!! ERROR !!!");
    }
    }

    label_handNumberValue->setText(
        QString::number(myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getID(), 10));
    label_gameNumberValue->setText(QString::number(myStartWindow->getSession()->getCurrentGame()->getGameID(), 10));
}

void GameTableWindow::refreshPot()
{
    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    textLabel_Sets->setText("$" + QString("%L1").arg(currentHand->getBoard()->getSets()));
    textLabel_Pot->setText("$" + QString("%L1").arg(currentHand->getBoard()->getPot()));
}

void GameTableWindow::guiUpdateDone()
{
    guiUpdateSemaphore.release();
}

void GameTableWindow::waitForGuiUpdateDone()
{
    guiUpdateSemaphore.acquire();
}

void GameTableWindow::dealHoleCards()
{

    int i, k;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        for (k = 0; k <= 1; k++)
        {
            holeCardsArray[i][k]->setFadeOutAction(false);
            holeCardsArray[i][k]->stopFlipCardsAnimation();
        }
    }

    QPixmap onePix = QPixmap::fromImage(QImage(myAppDataDir + "gfx/gui/misc/1px.png"));

    // TempArrays
    QPixmap tempCardsPixmapArray[2];
    int tempCardsIntArray[2];

    // Karten der Gegner und eigene Karten austeilen
    int j;
    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        (*it_c)->getCards(tempCardsIntArray);
        for (j = 0; j < 2; j++)
        {
            if ((*it_c)->getActiveStatus())
            {
                if (((*it_c)->getID() == 0) /* || DEBUG_MODE*/)
                {
                    tempCardsPixmapArray[j].load(myCardDeckStyle->getCurrentDir() +
                                                 QString::number(tempCardsIntArray[j], 10) + ".png");

                    holeCardsArray[(*it_c)->getID()][j]->setPixmap(tempCardsPixmapArray[j], false);
                    holeCardsArray[(*it_c)->getID()][j]->setFront(tempCardsPixmapArray[j]);
                }
                else
                {
                    holeCardsArray[(*it_c)->getID()][j]->setPixmap(flipside, true);
                    holeCardsArray[(*it_c)->getID()][j]->setFlipsidePix(flipside);
                }
            }
            else
            {

                holeCardsArray[(*it_c)->getID()][j]->setPixmap(onePix, false);
            }
        }
    }

    // refresh CardsChanceMonitor Tool
    refreshCardsChance(GAME_STATE_PREFLOP);
    refreshHandsRanges();
}

void GameTableWindow::dealBettingRoundCards(int myBettingRoundID)
{

    uncheckMyButtons();
    myButtonsCheckable(false);
    resetButtonsCheckStateMemory();
    clearMyButtons();

    horizontalSlider_bet->setDisabled(true);
    spinBox_betValue->setDisabled(true);

    switch (myBettingRoundID)
    {

    case 1:
    {
        dealFlopCards0();
    }
    break;
    case 2:
    {
        dealTurnCards0();
    }
    break;
    case 3:
    {
        dealRiverCards0();
    }
    break;
    default:
    {
        cout << "dealBettingRoundCards() Error" << endl;
    }
    }
}

void GameTableWindow::dealFlopCards0()
{
    dealFlopCards0Timer->start(preDealCardsSpeed);
}

void GameTableWindow::dealFlopCards1()
{

    boardCardsArray[0]->setPixmap(flipside, true);
    dealFlopCards1Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealFlopCards2()
{

    boardCardsArray[1]->setPixmap(flipside, true);
    dealFlopCards2Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealFlopCards3()
{

    boardCardsArray[2]->setPixmap(flipside, true);
    dealFlopCards3Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealFlopCards4()
{

    int tempBoardCardsArray[5];

    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getBoard()->getCards(tempBoardCardsArray);
    QPixmap card = QPixmap::fromImage(
        QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempBoardCardsArray[0], 10) + ".png"));

    boardCardsArray[0]->setFront(card);
    boardCardsArray[0]->setPixmap(card, false);

    dealFlopCards4Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealFlopCards5()
{

    int tempBoardCardsArray[5];
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getBoard()->getCards(tempBoardCardsArray);
    QPixmap card = QPixmap::fromImage(
        QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempBoardCardsArray[1], 10) + ".png"));

    boardCardsArray[1]->setFront(card);
    boardCardsArray[1]->setPixmap(card, false);

    dealFlopCards5Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealFlopCards6()
{

    int tempBoardCardsArray[5];
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getBoard()->getCards(tempBoardCardsArray);
    QPixmap card = QPixmap::fromImage(
        QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempBoardCardsArray[2], 10) + ".png"));

    boardCardsArray[2]->setFront(card);
    boardCardsArray[2]->setPixmap(card, false);

    // stable
    // wenn alle All In
    if (myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getAllInCondition())
    {
        dealFlopCards6Timer->start(AllInDealCardsSpeed);
    }
    // sonst normale Variante
    else
    {
        updateMyButtonsState(0); // mode 0 == called from dealBettingRoundcards
        dealFlopCards6Timer->start(postDealCardsSpeed);
    }

    // refresh CardsChanceMonitor Tool
    refreshCardsChance(GAME_STATE_FLOP);
    refreshHandsRanges();
}

void GameTableWindow::dealTurnCards0()
{
    dealTurnCards0Timer->start(preDealCardsSpeed);
}

void GameTableWindow::dealTurnCards1()
{

    boardCardsArray[3]->setPixmap(flipside, true);
    dealTurnCards1Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealTurnCards2()
{

    int tempBoardCardsArray[5];
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getBoard()->getCards(tempBoardCardsArray);
    QPixmap card = QPixmap::fromImage(
        QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempBoardCardsArray[3], 10) + ".png"));

    boardCardsArray[3]->setFront(card);
    boardCardsArray[3]->setPixmap(card, false);

    if (myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getAllInCondition())
    {
        dealTurnCards2Timer->start(AllInDealCardsSpeed);
    }
    // sonst normale Variante
    else
    {
        updateMyButtonsState(0); // mode 0 == called from dealBettingRoundcards
        dealTurnCards2Timer->start(postDealCardsSpeed);
    }
    // refresh CardsChanceMonitor Tool
    refreshCardsChance(GAME_STATE_TURN);
    refreshHandsRanges();
}

void GameTableWindow::dealRiverCards0()
{
    dealRiverCards0Timer->start(preDealCardsSpeed);
}

void GameTableWindow::dealRiverCards1()
{

    boardCardsArray[4]->setPixmap(flipside, true);

    // 	QTimer::singleShot(dealCardsSpeed, this, SLOT( dealRiverCards2() ));
    dealRiverCards1Timer->start(dealCardsSpeed);
}

void GameTableWindow::dealRiverCards2()
{

    int tempBoardCardsArray[5];
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getBoard()->getCards(tempBoardCardsArray);
    QPixmap card = QPixmap::fromImage(
        QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempBoardCardsArray[4], 10) + ".png"));

    boardCardsArray[4]->setFront(card);
    boardCardsArray[4]->setPixmap(card, false);

    if (myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getAllInCondition())
    {
        dealRiverCards2Timer->start(AllInDealCardsSpeed);
    }
    // sonst normale Variante
    else
    {
        updateMyButtonsState(0); // mode 0 == called from dealBettingRoundcards
        dealRiverCards2Timer->start(postDealCardsSpeed);
    }
    // refresh CardsChanceMonitor Tool
    refreshCardsChance(GAME_STATE_RIVER);
    refreshHandsRanges();
}

void GameTableWindow::provideMyActions(int mode)
{

    QString pushButtonFoldString;
    QString pushButtonBetRaiseString;
    QString pushButtonHalfPotBetRaiseString;
    QString pushButtonTwoThirdPotBetRaiseString;
    QString pushButtonPotBetRaiseString;
    QString lastPushButtonBetRaiseString = pushButton_BetRaise->text();
    QString pushButtonCallCheckString;
    QString pushButtonAllInString;
    QString lastPushButtonCallCheckString = pushButton_CallCheck->text();

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();
    PlayerList activePlayerList = currentHand->getActivePlayerList();

    // really disabled buttons if human player is fold/all-in or server-autofold... and not called from
    // dealBettingRoundcards
    if ((mode != 0 &&
         (humanPlayer->getAction() == PLAYER_ACTION_ALLIN || humanPlayer->getAction() == PLAYER_ACTION_FOLD ||
          (humanPlayer->getSet() == currentHand->getCurrentBettingRound()->getHighestSet() &&
           (humanPlayer->getAction() != PLAYER_ACTION_NONE)))) ||
        !humanPlayer->isSessionActive() /*server-autofold*/)
    {

        pushButton_BetRaise->setText("");
        pushButton_BetRaiseHalfPot->setText("50% pot");
        pushButton_BetRaiseTwoThirdPot->setText("75% pot");
        pushButton_BetRaisePot->setText("100% pot");
        pushButton_CallCheck->setText("");
        pushButton_Fold->setText("");
        pushButton_AllIn->setText("");

        horizontalSlider_bet->setDisabled(true);
        spinBox_betValue->setDisabled(true);

        myButtonsCheckable(false);
    }
    else
    {
        horizontalSlider_bet->setEnabled(true);
        spinBox_betValue->setEnabled(true);

        // show available actions on buttons
        if (currentHand->getCurrentRound() == 0)
        { // preflop

            if (humanPlayer->getCash() + humanPlayer->getSet() >
                    currentHand->getCurrentBettingRound()->getHighestSet() &&
                !currentHand->getCurrentBettingRound()->getFullBetRule())
            {
                pushButtonBetRaiseString = RaiseString + "\n$" + QString("%L1").arg(getBetAmount());
            }

            if (humanPlayer->getSet() == currentHand->getCurrentBettingRound()->getHighestSet() &&
                humanPlayer->getButton() == 3)
            {
                pushButtonCallCheckString = CheckString;
            }
            else
            {
                pushButtonCallCheckString = CallString + "\n$" + QString("%L1").arg(getCallAmount());
            }

            pushButtonFoldString = FoldString;
            if (!currentHand->getCurrentBettingRound()->getFullBetRule())
            {
                pushButtonAllInString = AllInString;
            }
        }
        else
        { // flop,turn,river

            if (currentHand->getCurrentBettingRound()->getHighestSet() == 0 && pushButton_Fold->isCheckable())
            {
                pushButtonFoldString = CheckString + " /\n" + FoldString;
            }
            else
            {
                pushButtonFoldString = FoldString;
            }
            if (currentHand->getCurrentBettingRound()->getHighestSet() == 0)
            {

                pushButtonCallCheckString = CheckString;
                pushButtonBetRaiseString = BetString + "\n$" + QString("%L1").arg(getBetAmount());
            }
            if (currentHand->getCurrentBettingRound()->getHighestSet() > 0 &&
                currentHand->getCurrentBettingRound()->getHighestSet() > humanPlayer->getSet())
            {
                pushButtonCallCheckString = CallString + "\n$" + QString("%L1").arg(getCallAmount());
                if (humanPlayer->getCash() + humanPlayer->getSet() >
                        currentHand->getCurrentBettingRound()->getHighestSet() &&
                    !currentHand->getCurrentBettingRound()->getFullBetRule())
                {
                    pushButtonBetRaiseString = RaiseString + "\n$" + QString("%L1").arg(getBetAmount());
                }
            }
            if (!currentHand->getCurrentBettingRound()->getFullBetRule())
            {
                pushButtonAllInString = AllInString;
            }
        }

        if (mode == 0)
        {
            if (humanPlayer->getAction() != PLAYER_ACTION_FOLD)
            {
                pushButtonBetRaiseString = BetString + "\n$" + QString("%L1").arg(getBetAmount());
                pushButtonCallCheckString = CheckString;
                if ((activePlayerList->size() > 2 && humanPlayer->getButton() == BUTTON_SMALL_BLIND) ||
                    (activePlayerList->size() <= 2 && humanPlayer->getButton() == BUTTON_BIG_BLIND))
                {
                    pushButtonFoldString = FoldString;
                }
                else
                {
                    pushButtonFoldString = CheckString + " /\n" + FoldString;
                }

                pushButtonAllInString = AllInString;
            }
            else
            {
                pushButtonBetRaiseString = "";
                pushButtonCallCheckString = "";
                pushButtonFoldString = "";
                pushButtonAllInString = "";
                horizontalSlider_bet->setDisabled(true);
                spinBox_betValue->setDisabled(true);

                myButtonsCheckable(false);
            }
        }

        // if text changed on checked button --> do something to prevent unwanted actions
        if (pushButtonCallCheckString != lastPushButtonCallCheckString)
        {

            if (pushButton_CallCheck->isChecked())
            {
                // uncheck a previous checked button to prevent unwanted action
                uncheckMyButtons();
                resetButtonsCheckStateMemory();
            }
        }

        if (pushButtonBetRaiseString == "")
        {

            horizontalSlider_bet->setDisabled(true);
            spinBox_betValue->setDisabled(true);
        }

        pushButton_Fold->setText(pushButtonFoldString);
        pushButton_BetRaise->setText(pushButtonBetRaiseString);
        pushButton_BetRaiseHalfPot->setText("50% pot");
        pushButton_BetRaiseTwoThirdPot->setText("75% pot");
        pushButton_BetRaisePot->setText("100% pot");
        pushButton_CallCheck->setText(pushButtonCallCheckString);
        pushButton_AllIn->setText(pushButtonAllInString);

        if (pushButton_BetRaise->text().startsWith(RaiseString))
        {

            horizontalSlider_bet->setMinimum(currentHand->getCurrentBettingRound()->getHighestSet() -
                                             humanPlayer->getSet() +
                                             currentHand->getCurrentBettingRound()->getMinimumRaise());
            horizontalSlider_bet->setMaximum(humanPlayer->getCash());
            horizontalSlider_bet->setSingleStep(10);
            spinBox_betValue->setMinimum(currentHand->getCurrentBettingRound()->getHighestSet() -
                                         humanPlayer->getSet() +
                                         currentHand->getCurrentBettingRound()->getMinimumRaise());
            spinBox_betValue->setMaximum(humanPlayer->getCash());
            changeSpinBoxBetValue(horizontalSlider_bet->value());

            myActionIsRaise = 1;
        }
        else if (pushButton_BetRaise->text().startsWith(BetString))
        {

            horizontalSlider_bet->setMinimum(currentHand->getSmallBlind() * 2);
            horizontalSlider_bet->setMaximum(humanPlayer->getCash());
            horizontalSlider_bet->setSingleStep(10);
            spinBox_betValue->setMinimum(currentHand->getSmallBlind() * 2);
            spinBox_betValue->setMaximum(humanPlayer->getCash());
            changeSpinBoxBetValue(horizontalSlider_bet->value());

            myActionIsBet = 1;
        }

        // if value changed on bet/raise button --> uncheck to prevent unwanted actions
        int lastBetValue = lastPushButtonBetRaiseString.simplified().remove(QRegularExpression("[^0-9]")).toInt();

        if ((lastBetValue < horizontalSlider_bet->minimum() && pushButton_BetRaise->isChecked()))
        {

            uncheckMyButtons();
            resetButtonsCheckStateMemory();
        }

        spinBox_betValue->setFocus();
        spinBox_betValue->selectAll();
    }
}

void GameTableWindow::meInAction()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getGuiInterface()->showCards(0);

    myButtonsCheckable(false);

    GameState currentState =
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->getBettingRoundID();
    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();
    if (currentState == GAME_STATE_PREFLOP)
        humanPlayer->setPreflopPotOdd(humanPlayer->getPotOdd());

    horizontalSlider_bet->setEnabled(true);
    spinBox_betValue->setEnabled(true);

    myActionIsRaise = 0;
    myActionIsBet = 0;

    QString lastPushButtonFoldString = pushButton_Fold->text();

    // paint actions on buttons
    provideMyActions();

    // do remembered action
    if (pushButtonBetRaiseIsChecked)
    {
        pushButton_BetRaise->click();
        pushButtonBetRaiseIsChecked = false;
    }
    if (pushButtonCallCheckIsChecked)
    {
        pushButton_CallCheck->click();
        pushButtonCallCheckIsChecked = false;
    }
    if (pushButtonFoldIsChecked)
    {
        if (lastPushButtonFoldString == CheckString + " /\n" + FoldString &&
            pushButton_CallCheck->text() == CheckString)
        {
            pushButton_CallCheck->click();
        }
        else
        {
            pushButton_Fold->click();
        }
        pushButtonFoldIsChecked = false;
    }
    if (pushButtonAllInIsChecked)
    {
        pushButton_AllIn->click();
        pushButtonAllInIsChecked = false;
    }

    // automatic mode
    switch (playingMode)
    {
    case 0: // Manual mode
        break;
    case 1: // Auto check / call all
        myCallCheck();
        break;
    case 2: // Auto check / fold all
        if (pushButton_CallCheck->text() == CheckString)
        {
            myCheck();
        }
        else
        {
            myFold();
        }
        break;
    }
}

void GameTableWindow::disableMyButtons()
{

    std::shared_ptr<Player> humanPlayer =
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getSeatsList()->front();

    clearMyButtons();

    // clear userWidgets
    horizontalSlider_bet->setDisabled(true);
    spinBox_betValue->setDisabled(true);
    horizontalSlider_bet->setMinimum(0);
    horizontalSlider_bet->setMaximum(humanPlayer->getCash());
    spinBox_betValue->setMinimum(0);
    spinBox_betValue->setMaximum(humanPlayer->getCash());
    spinBox_betValue->clear();
    horizontalSlider_bet->setValue(0);

#ifdef _WIN32
    QString humanPlayerButtonFontSize = "10";
#else
    QString humanPlayerButtonFontSize = "12";
#endif
}

void GameTableWindow::myCallCheck()
{

    if (pushButton_CallCheck->text().startsWith(CallString))
    {
        myCall();
    }
    if (pushButton_CallCheck->text() == CheckString)
    {
        myCheck();
    }
}

void GameTableWindow::myFold()
{

    if (pushButton_Fold->text() == FoldString)
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();
        humanPlayer->setAction(PLAYER_ACTION_FOLD);
        humanPlayer->setTurn(0);

        // set that i was the last active player. need this for unhighlighting groupbox
        currentHand->setPreviousPlayerID(0);

        // 		statusBar()->clearMessage();

        // Spiel läuft weiter
        myActionDone();
    }
}

void GameTableWindow::myCheck()
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();
    humanPlayer->setTurn(0);
    humanPlayer->setAction(PLAYER_ACTION_CHECK);

    // set that i was the last active player. need this for unhighlighting groupbox
    currentHand->setPreviousPlayerID(0);

    // 	statusBar()->clearMessage();

    // Spiel läuft weiter
    myActionDone();
}

int GameTableWindow::getCallAmount()
{

    int tempHighestSet = 0;

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();
    tempHighestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    if (humanPlayer->getCash() + humanPlayer->getSet() <= tempHighestSet)
    {

        return humanPlayer->getCash();
    }
    else
    {
        return tempHighestSet - humanPlayer->getSet();
    }
}

int GameTableWindow::getBetRaisePushButtonValue()
{

    int betValue = pushButton_BetRaise->text().simplified().remove(QRegularExpression("[^0-9]")).toInt();
    return betValue;
}

int GameTableWindow::getBetAmount()
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();

    int betValue = getBetRaisePushButtonValue();
    int minimum;

    minimum = currentHand->getCurrentBettingRound()->getHighestSet() - humanPlayer->getSet() +
              currentHand->getCurrentBettingRound()->getMinimumRaise();

    if (betValue < minimum)
    {
        return min(minimum, humanPlayer->getCash());
    }
    else
    {
        return betValue;
    }
}

void GameTableWindow::myCall()
{

    int tempHighestSet = 0;
    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();

    tempHighestSet = currentHand->getCurrentBettingRound()->getHighestSet();

    if (humanPlayer->getCash() + humanPlayer->getSet() <= tempHighestSet)
    {

        humanPlayer->setSet(humanPlayer->getCash());
        humanPlayer->setCash(0);
        humanPlayer->setAction(PLAYER_ACTION_ALLIN);
    }
    else
    {
        humanPlayer->setSet(tempHighestSet - humanPlayer->getSet());
        humanPlayer->setAction(PLAYER_ACTION_CALL);
    }
    humanPlayer->setTurn(0);

    currentHand->getBoard()->collectSets();
    refreshPot();

    // set that i was the last active player. need this for unhighlighting groupbox
    currentHand->setPreviousPlayerID(0);

    // 	statusBar()->clearMessage();

    // Spiel läuft weiter
    myActionDone();
}

void GameTableWindow::mySet()
{
    GameState currentState =
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->getBettingRoundID();

    if (pushButton_BetRaise->text() != "")
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();

        int tempCash = humanPlayer->getCash();

        // 		cout << "Set-Value " << getBetRaisePushButtonValue() << endl;
        humanPlayer->setSet(getBetRaisePushButtonValue());

        if (getBetRaisePushButtonValue() >= tempCash)
        {

            humanPlayer->setSet(humanPlayer->getCash());
            humanPlayer->setCash(0);
            humanPlayer->setAction(PLAYER_ACTION_ALLIN);

            // full bet rule
            if (currentHand->getCurrentBettingRound()->getHighestSet() +
                    currentHand->getCurrentBettingRound()->getMinimumRaise() >
                humanPlayer->getSet())
            {
                currentHand->getCurrentBettingRound()->setFullBetRule(true);
            }
        }

        if (myActionIsRaise)
        {
            // do not if allIn
            if (humanPlayer->getAction() != 6)
            {
                humanPlayer->setAction(PLAYER_ACTION_RAISE);
            }
            myActionIsRaise = 0;

            currentHand->getCurrentBettingRound()->setMinimumRaise(
                humanPlayer->getSet() - currentHand->getCurrentBettingRound()->getHighestSet());
        }

        if (myActionIsBet)
        {
            // do not if allIn
            if (humanPlayer->getAction() != 6)
            {
                humanPlayer->setAction(PLAYER_ACTION_BET);
            }
            myActionIsBet = 0;

            currentHand->getCurrentBettingRound()->setMinimumRaise(humanPlayer->getSet());
        }

        currentHand->getCurrentBettingRound()->setHighestSet(humanPlayer->getSet());

        humanPlayer->setTurn(0);

        currentHand->getBoard()->collectSets();
        refreshPot();

        // 		statusBar()->clearMessage();

        // set that i was the last active player. need this for unhighlighting groupbox
        currentHand->setPreviousPlayerID(0);

        // lastPlayerAction für Karten umblättern reihenfolge setzrn
        currentHand->setLastActionPlayerID(humanPlayer->getID());

        // Spiel läuft weiter
        myActionDone();
    }
}

void GameTableWindow::myAllIn()
{

    if (pushButton_AllIn->text() == AllInString)
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        std::shared_ptr<Player> humanPlayer = currentHand->getSeatsList()->front();

        humanPlayer->setSet(humanPlayer->getCash());
        humanPlayer->setCash(0);
        humanPlayer->setAction(PLAYER_ACTION_ALLIN);

        // full bet rule
        if (currentHand->getCurrentBettingRound()->getHighestSet() +
                currentHand->getCurrentBettingRound()->getMinimumRaise() >
            humanPlayer->getSet())
        {
            currentHand->getCurrentBettingRound()->setFullBetRule(true);
        }

        if (humanPlayer->getSet() > currentHand->getCurrentBettingRound()->getHighestSet())
        {
            currentHand->getCurrentBettingRound()->setMinimumRaise(
                humanPlayer->getSet() - currentHand->getCurrentBettingRound()->getHighestSet());

            currentHand->getCurrentBettingRound()->setHighestSet(humanPlayer->getSet());

            // lastPlayerAction für Karten umblättern reihenfolge setzrn
            currentHand->setLastActionPlayerID(humanPlayer->getID());
        }

        humanPlayer->setTurn(0);

        currentHand->getBoard()->collectSets();
        refreshPot();

        // set that i was the last active player. need this for unhighlighting groupbox
        currentHand->setPreviousPlayerID(0);

        // Spiel läuft weiter
        myActionDone();
    }
}

void GameTableWindow::pushButtonBetRaiseClicked(bool checked)
{

    if (pushButton_BetRaise->isCheckable())
    {
        if (checked)
        {
            pushButton_CallCheck->setChecked(false);
            pushButton_Fold->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonCallCheckIsChecked = false;
            pushButtonFoldIsChecked = false;
            pushButtonAllInIsChecked = false;

            pushButtonBetRaiseIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonBetRaiseIsChecked = false;
            myLastPreActionBetValue = 0;
        }
    }
    else
    {
        mySet();
    }
}
void GameTableWindow::pushButtonBetRaiseHalfPotClicked(bool checked)
{

    if (pushButton_BetRaiseHalfPot->isCheckable())
    {
        if (checked)
        {
            pushButton_BetRaiseTwoThirdPot->setChecked(false);
            pushButton_BetRaisePot->setChecked(false);
            pushButton_CallCheck->setChecked(false);
            pushButton_Fold->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonCallCheckIsChecked = false;
            pushButtonFoldIsChecked = false;
            pushButtonAllInIsChecked = false;

            pushButtonBetRaiseHalfPotIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonBetRaiseHalfPotIsChecked = false;
            myLastPreActionBetValue = 0;
        }
    }
    else
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        int amount = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() +
                     currentHand->getCurrentBettingRound()->getHighestSet();

        amount = amount / 2;

        pushButton_BetRaise->setText("");
        QString betRaise = currentHand->getCurrentBettingRound()->getHighestSet() == 0 ? BetString : RaiseString;
        pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(amount));
        mySet();
    }
}
void GameTableWindow::pushButtonBetRaiseTwoThirdPotClicked(bool checked)
{

    if (pushButton_BetRaiseTwoThirdPot->isCheckable())
    {
        if (checked)
        {
            pushButton_BetRaiseHalfPot->setChecked(false);
            pushButton_BetRaisePot->setChecked(false);
            pushButton_CallCheck->setChecked(false);
            pushButton_Fold->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonCallCheckIsChecked = false;
            pushButtonFoldIsChecked = false;
            pushButtonAllInIsChecked = false;

            pushButtonBetRaiseTwoThirdPotIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonBetRaiseTwoThirdPotIsChecked = false;
            myLastPreActionBetValue = 0;
        }
    }
    else
    {
        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        int amount = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() +
                     currentHand->getCurrentBettingRound()->getHighestSet();

        amount = amount * 0.75;

        pushButton_BetRaise->setText("");
        QString betRaise = currentHand->getCurrentBettingRound()->getHighestSet() == 0 ? BetString : RaiseString;
        pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(amount));
        mySet();
    }
}
void GameTableWindow::pushButtonBetRaisePotClicked(bool checked)
{

    if (pushButton_BetRaisePot->isCheckable())
    {
        if (checked)
        {
            pushButton_BetRaiseHalfPot->setChecked(false);
            pushButton_BetRaiseTwoThirdPot->setChecked(false);
            pushButton_CallCheck->setChecked(false);
            pushButton_Fold->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonCallCheckIsChecked = false;
            pushButtonFoldIsChecked = false;
            pushButtonAllInIsChecked = false;

            pushButtonBetRaisePotIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonBetRaisePotIsChecked = false;
            myLastPreActionBetValue = 0;
        }
    }
    else
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        int amount = currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets() +
                     currentHand->getCurrentBettingRound()->getHighestSet();

        pushButton_BetRaise->setText("");
        QString betRaise = currentHand->getCurrentBettingRound()->getHighestSet() == 0 ? BetString : RaiseString;
        pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(amount));
        mySet();
    }
}
void GameTableWindow::pushButtonCallCheckClicked(bool checked)
{

    if (pushButton_CallCheck->isCheckable())
    {
        if (checked)
        {
            pushButton_Fold->setChecked(false);
            pushButton_BetRaise->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonAllInIsChecked = false;
            pushButtonFoldIsChecked = false;
            pushButtonBetRaiseIsChecked = false;

            pushButtonCallCheckIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonCallCheckIsChecked = false;
        }
    }
    else
    {
        myCallCheck();
    }
}

void GameTableWindow::pushButtonFoldClicked(bool checked)
{

    if (pushButton_Fold->isCheckable())
    {
        if (checked)
        {
            pushButton_CallCheck->setChecked(false);
            pushButton_BetRaise->setChecked(false);
            pushButton_AllIn->setChecked(false);

            pushButtonAllInIsChecked = false;
            pushButtonCallCheckIsChecked = false;
            pushButtonBetRaiseIsChecked = false;

            pushButtonFoldIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonFoldIsChecked = false;
        }
    }
    else
    {
        myFold();
    }
}

void GameTableWindow::pushButtonAllInClicked(bool checked)
{

    if (pushButton_AllIn->isCheckable())
    {
        if (checked)
        {
            pushButton_CallCheck->setChecked(false);
            pushButton_BetRaise->setChecked(false);
            pushButton_Fold->setChecked(false);

            pushButtonFoldIsChecked = false;
            pushButtonCallCheckIsChecked = false;
            pushButtonBetRaiseIsChecked = false;

            pushButtonAllInIsChecked = true;

            if (!radioButton_manualAction->isChecked())
                radioButton_manualAction->click();
        }
        else
        {
            pushButtonAllInIsChecked = false;
        }
    }
    else
    {
        myAllIn();
    }
}

void GameTableWindow::myActionDone()
{
    std::shared_ptr<Player> humanPlayer =
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getSeatsList()->front();
    GameState currentState =
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->getBettingRoundID();
    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    const int nbPlayers = currentHand->getActivePlayerList()->size();

#ifdef LOG_POKER_EXEC

    cout << endl
         << "\t" << humanPlayer->getPositionLabel(humanPlayer->getPosition()) << "\tHuman player"
         << "\t" << humanPlayer->getCardsValueString() << "\t" << "stack = " << humanPlayer->getCash()
         << ", pot = " << currentHand->getBoard()->getPot() + currentHand->getBoard()->getSets()
         << "\tPFR : " << humanPlayer->getStatistics(nbPlayers).getPreflopStatistics().getPreflopRaise() << endl
         << endl;

#endif

    if (currentState == GAME_STATE_PREFLOP)
    {
        humanPlayer->getCurrentHandActions().getPreflopActions().push_back(humanPlayer->getAction());
        if (humanPlayer->getAction() == PLAYER_ACTION_RAISE || humanPlayer->getAction() == PLAYER_ACTION_ALLIN)
            currentHand->setPreflopLastRaiserID(humanPlayer->getID());
        humanPlayer->updatePreflopStatistics();
        if (humanPlayer->getAction() != PLAYER_ACTION_FOLD)
            humanPlayer->updateUnplausibleRangesGivenPreflopActions();
    }
    else if (currentState == GAME_STATE_FLOP)
    {
        humanPlayer->getCurrentHandActions().getFlopActions().push_back(humanPlayer->getAction());
        humanPlayer->updateFlopStatistics();
        humanPlayer->updateUnplausibleRangesGivenFlopActions();
    }
    else if (currentState == GAME_STATE_TURN)
    {
        humanPlayer->getCurrentHandActions().getTurnActions().push_back(humanPlayer->getAction());
        humanPlayer->updateTurnStatistics();
        humanPlayer->updateUnplausibleRangesGivenTurnActions();
    }
    else if (currentState == GAME_STATE_RIVER)
    {
        humanPlayer->getCurrentHandActions().getRiverActions().push_back(humanPlayer->getAction());
        humanPlayer->updateRiverStatistics();
        humanPlayer->updateUnplausibleRangesGivenRiverActions();
    }

    disableMyButtons();

    nextPlayerAnimation();

    // prevent escape button working while allIn
    myActionIsRaise = 0;
    myActionIsBet = 0;

#ifdef LOG_POKER_EXEC

    PlayerAction myAction = humanPlayer->getAction();

    if (myAction == PLAYER_ACTION_FOLD)
        cout << "FOLD";
    else if (myAction == PLAYER_ACTION_BET)
        cout << "BET ";
    else if (myAction == PLAYER_ACTION_RAISE)
        cout << "RAISE ";
    else if (myAction == PLAYER_ACTION_CALL)
        cout << "CALL ";
    else if (myAction == PLAYER_ACTION_CHECK)
        cout << "CHECK";
    else if (myAction == PLAYER_ACTION_ALLIN)
        cout << "ALLIN ";
    else if (myAction == PLAYER_ACTION_NONE)
        cout << "NONE";
    else
        cout << "undefined ?";

    cout << endl
         << endl
         << "---------------------------------------------------------------------------------" << endl
         << endl;
#endif
}

void GameTableWindow::nextPlayerAnimation()
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    // refresh Change Player
    refreshSet();

    PlayerListConstIterator it_c;
    PlayerList seatsList = currentHand->getSeatsList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        if ((*it_c)->getID() == currentHand->getPreviousPlayerID())
            break;
    }

    if (currentHand->getPreviousPlayerID() != -1)
    {
        refreshAction(currentHand->getPreviousPlayerID(), (*it_c)->getAction());
    }
    refreshCash();

    refreshHandsRanges();

    // refresh actions for human player
    updateMyButtonsState();

    nextPlayerAnimationTimer->start(nextPlayerSpeed1);
}

void GameTableWindow::bettingRoundAnimation(int myBettingRoundID)
{

    switch (myBettingRoundID)
    {

    case 0:
    {
        preflopAnimation2();
    }
    break;
    case 1:
    {
        flopAnimation2();
    }
    break;
    case 2:
    {
        turnAnimation2();
    }
    break;
    case 3:
    {
        riverAnimation2();
    }
    break;
    default:
    {
        cout << "bettingRoundAnimation() Error" << endl;
    }
    }
}

void GameTableWindow::preflopAnimation1()
{
    preflopAnimation1Timer->start(nextPlayerSpeed2);
}
void GameTableWindow::preflopAnimation1Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->run();
}

void GameTableWindow::preflopAnimation2()
{
    preflopAnimation2Timer->start(preflopNextPlayerSpeed);
}
void GameTableWindow::preflopAnimation2Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->nextPlayer();
}

void GameTableWindow::flopAnimation1()
{
    flopAnimation1Timer->start(nextPlayerSpeed2);
}
void GameTableWindow::flopAnimation1Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->run();
}

void GameTableWindow::flopAnimation2()
{
    flopAnimation2Timer->start(nextPlayerSpeed3);
}
void GameTableWindow::flopAnimation2Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->nextPlayer();
}

void GameTableWindow::turnAnimation1()
{
    turnAnimation1Timer->start(nextPlayerSpeed2);
}
void GameTableWindow::turnAnimation1Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->run();
}

void GameTableWindow::turnAnimation2()
{
    turnAnimation2Timer->start(nextPlayerSpeed3);
}
void GameTableWindow::turnAnimation2Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->nextPlayer();
}

void GameTableWindow::riverAnimation1()
{
    riverAnimation1Timer->start(nextPlayerSpeed2);
}
void GameTableWindow::riverAnimation1Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->run();
}

void GameTableWindow::riverAnimation2()
{
    riverAnimation2Timer->start(nextPlayerSpeed3);
}
void GameTableWindow::riverAnimation2Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->nextPlayer();
}

void GameTableWindow::postRiverAnimation1()
{
    postRiverAnimation1Timer->start(nextPlayerSpeed2);
}
void GameTableWindow::postRiverAnimation1Action()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBettingRound()->postRiverRun();
}

void GameTableWindow::postRiverRunAnimation1()
{
    postRiverRunAnimation1Timer->start(postRiverRunAnimationSpeed);
}

void GameTableWindow::postRiverRunAnimation2()
{

    uncheckMyButtons();
    myButtonsCheckable(false);
    clearMyButtons();
    resetButtonsCheckStateMemory();

    horizontalSlider_bet->setDisabled(true);
    spinBox_betValue->setDisabled(true);

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    int nonfoldPlayersCounter = 0;
    PlayerListConstIterator it_c;
    PlayerList activePlayerList = currentGame->getActivePlayerList();
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
            nonfoldPlayersCounter++;
    }

    if (nonfoldPlayersCounter != 1)
    {

        if (!flipHolecardsAllInAlreadyDone)
        {

            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {
                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD && (*it_c)->checkIfINeedToShowCards())
                {

                    showHoleCards((*it_c)->getID());
                }
            }
            // Wenn einmal umgedreht dann fertig!!
            flipHolecardsAllInAlreadyDone = true;
        }
        else
        {
            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {
                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
                {
                    // set Player value (logging) for all in already shown cards
                    (*it_c)->setCardsFlip(1, 3);
                }
            }
        }
        postRiverRunAnimation2Timer->start(postRiverRunAnimationSpeed);
    }
    else
    {

        postRiverRunAnimation3();
    }
}

void GameTableWindow::postRiverRunAnimation3()
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    int nonfoldPlayerCounter = 0;
    PlayerListConstIterator it_c;

    PlayerList activePlayerList = currentHand->getActivePlayerList();
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
        {
            nonfoldPlayerCounter++;
        }
    }

    list<unsigned> winners = currentHand->getBoard()->getWinners();

    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD &&
            (*it_c)->getCardsValueInt() == currentHand->getCurrentBettingRound()->getHighestCardsValue())
        {

            // Show "Winner" label
            actionLabelArray[(*it_c)->getID()]->setPixmap(
                QPixmap::fromImage(QImage(myGameTableStyle->getActionPic(7))));

            // set Player value (logging)
            myGuiDisplayGameActions->logPlayerWinsMsg(QString::fromUtf8((*it_c)->getName().c_str()),
                                                      (*it_c)->getLastMoneyWon(), true);
        }
    }

    // log side pot winners -> TODO
    list<unsigned>::iterator it_int;
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getAction() != PLAYER_ACTION_FOLD &&
            (*it_c)->getCardsValueInt() != currentHand->getCurrentBettingRound()->getHighestCardsValue())
        {

            for (it_int = winners.begin(); it_int != winners.end(); ++it_int)
            {
                if ((*it_int) == (*it_c)->getID())
                {
                    myGuiDisplayGameActions->logPlayerWinsMsg(QString::fromUtf8((*it_c)->getName().c_str()),
                                                              (*it_c)->getLastMoneyWon(), false);
                }
            }
        }
    }

    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {
        if ((*it_c)->getCash() == 0)
        {
            //			currentHand->getGuiInterface()->logPlayerSitsOut((*it_c)->getName());
        }
    }

    //	textBrowser_Log->append("");

    postRiverRunAnimation3Timer->start(postRiverRunAnimationSpeed / 2);
}

void GameTableWindow::postRiverRunAnimation4()
{

    distributePotAnimCounter = 0;
    potDistributeTimer->start(winnerBlinkSpeed);
}

void GameTableWindow::postRiverRunAnimation5()
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    PlayerList activePlayerList = currentHand->getActivePlayerList();
    PlayerListConstIterator it_c;

    if (distributePotAnimCounter < 10)
    {

        if (distributePotAnimCounter == 0 || distributePotAnimCounter == 2 || distributePotAnimCounter == 4 ||
            distributePotAnimCounter == 6 || distributePotAnimCounter == 8)
        {

            label_Pot->setText("");

            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {

                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD &&
                    (*it_c)->getCardsValueInt() == currentHand->getCurrentBettingRound()->getHighestCardsValue())
                {

                    playerNameLabelArray[(*it_c)->getID()]->hide();
                }
            }
        }
        else
        {
            label_Pot->setText(PotString);

            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {

                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD &&
                    (*it_c)->getCardsValueInt() == currentHand->getCurrentBettingRound()->getHighestCardsValue())
                {

                    playerNameLabelArray[(*it_c)->getID()]->show();
                }
            }
        }

        distributePotAnimCounter++;
    }
    else
    {
        potDistributeTimer->stop();
        postRiverRunAnimation5Timer->start(gameSpeed);
    }
}

void GameTableWindow::postRiverRunAnimation6()
{
    int i;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        playerNameLabelArray[i]->show();
    }

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    refreshCash();
    refreshPot();

    // if 1 active playerleft, open "new game" dialog
    int playersPositiveCashCounter = 0;

    PlayerListConstIterator it_c;
    PlayerList activePlayerList = currentHand->getActivePlayerList();
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {

        if ((*it_c)->getCash() > 0)
            playersPositiveCashCounter++;
    }

    if (playersPositiveCashCounter == 1)
    {

        for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
        {

            if ((*it_c)->getCash() > 0)
            {
                currentHand->getGuiInterface()->logPlayerWinGame(
                    (*it_c)->getName(), myStartWindow->getSession()->getCurrentGame()->getGameID());
            }
        }

        currentGameOver = true;

        pushButton_break->setDisabled(false);
        QFontMetrics tempMetrics = this->fontMetrics();
        int width = tempMetrics.horizontalAdvance(tr("Start"));

        pushButton_break->setMinimumSize(width + 10, 20);
        pushButton_break->setText(tr("Start"));
        blinkingStartButtonAnimationTimer->start(500);

        return;
    }

    postRiverRunAnimation6Timer->start(newRoundSpeed);
}

void GameTableWindow::showHoleCards(unsigned playerId, bool allIn)
{
    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
    // TempArrays
    QPixmap tempCardsPixmapArray[2];
    int tempCardsIntArray[2];
    int j;
    PlayerListConstIterator it_c;
    PlayerList activePlayerList = currentHand->getActivePlayerList();
    for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
    {

        if ((*it_c)->getID() == playerId)
        {

            (*it_c)->getCards(tempCardsIntArray);
            for (j = 0; j < 2; j++)
            {

                tempCardsPixmapArray[j] = QPixmap::fromImage(
                    QImage(myCardDeckStyle->getCurrentDir() + QString::number(tempCardsIntArray[j], 10) + ".png"));
                holeCardsArray[(*it_c)->getID()][j]->setPixmap(tempCardsPixmapArray[j], false);
            }
            // set Player value (logging)
            if (currentHand->getCurrentRound() < GAME_STATE_RIVER || allIn)
            {
                (*it_c)->setCardsFlip(1, 2); // for BettingRound before postriver or allin just log the hole cards
            }
            else
            {
                (*it_c)->setCardsFlip(1, 1); // for postriver log the value
            }
        }
    }
}

void GameTableWindow::flipHolecardsAllIn()
{

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();

    if (!flipHolecardsAllInAlreadyDone && currentGame->getCurrentHand()->getCurrentRound() < GAME_STATE_RIVER)
    {
        // Aktive Spieler zählen --> wenn nur noch einer nicht-folded dann keine Karten umdrehen
        int nonfoldPlayersCounter = 0;
        PlayerListConstIterator it_c;
        PlayerList activePlayerList = currentGame->getActivePlayerList();
        for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
        {
            if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
                nonfoldPlayersCounter++;
        }

        if (nonfoldPlayersCounter != 1)
        {
            for (it_c = activePlayerList->begin(); it_c != activePlayerList->end(); ++it_c)
            {
                if ((*it_c)->getAction() != PLAYER_ACTION_FOLD)
                {

                    showHoleCards((*it_c)->getID());
                }
            }
        }

        // Wenn einmal umgedreht dann fertig!!
        flipHolecardsAllInAlreadyDone = true;
    }
}

void GameTableWindow::startNewHand()
{
    if (!breakAfterCurrentHand)
    {
        myStartWindow->getSession()->getCurrentGame()->initHand();
        myStartWindow->getSession()->getCurrentGame()->startHand();
    }
    else
    {

        pushButton_break->setDisabled(false);

        QFontMetrics tempMetrics = this->fontMetrics();
        int width = tempMetrics.horizontalAdvance(tr("Start"));
        pushButton_break->setMinimumSize(width + 10, 20);
        pushButton_break->setText(tr("Start"));
        breakAfterCurrentHand = false;
        blinkingStartButtonAnimationTimer->start(500);
    }
}

void GameTableWindow::handSwitchRounds()
{
    myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->switchRounds();
}

void GameTableWindow::nextRoundCleanGui()
{

    int i, j;

    QPixmap onePix = QPixmap::fromImage(QImage(myAppDataDir + "gfx/gui/misc/1px.png"));
    for (i = 0; i < 5; i++)
    {
        boardCardsArray[i]->setPixmap(onePix, false);
        boardCardsArray[i]->setFadeOutAction(false);
        boardCardsArray[i]->stopFlipCardsAnimation();
    }
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        for (j = 0; j <= 1; j++)
        {
            holeCardsArray[i][j]->setFadeOutAction(false);
            holeCardsArray[i][j]->stopFlipCardsAnimation();
        }
    }

    // for startNewGame during human player is active
    if (myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->getSeatsList()->front()->getActiveStatus() ==
        1)
    {
        disableMyButtons();
    }

    textLabel_handLabel->setText("");

    refreshSet();
    refreshButton();

    std::shared_ptr<Game> currentGame = myStartWindow->getSession()->getCurrentGame();
    PlayerListConstIterator it_c;
    PlayerList seatsList = currentGame->getSeatsList();
    for (it_c = seatsList->begin(); it_c != seatsList->end(); ++it_c)
    {
        refreshAction((*it_c)->getID(), (*it_c)->getAction());
    }

    refreshCash();
    refreshPlayersActiveInactiveStyles();
    refreshPlayerName();
    refreshPlayerStatistics();
    refreshHandsRanges();

    flipHolecardsAllInAlreadyDone = false;

    pushButton_break->setEnabled(true);
    breakAfterCurrentHand = false;

    // Clean breakbutton
    blinkingStartButtonAnimationTimer->stop();
    myGameTableStyle->setBreakButtonStyle(pushButton_break, 0);

    blinkingStartButtonAnimationTimer->stop();
    QFontMetrics tempMetrics = this->fontMetrics();
    int width = tempMetrics.horizontalAdvance(tr("Show cards"));
    pushButton_break->setMinimumSize(width + 10, 20);
    pushButton_break->setText(tr("Show cards"));

    horizontalSlider_bet->setDisabled(true);
    spinBox_betValue->setDisabled(true);

    uncheckMyButtons();
    myButtonsCheckable(false);
    resetButtonsCheckStateMemory();
    clearMyButtons();
}

void GameTableWindow::stopTimer()
{

    dealFlopCards0Timer->stop();
    dealFlopCards1Timer->stop();
    dealFlopCards2Timer->stop();
    dealFlopCards3Timer->stop();
    dealFlopCards4Timer->stop();
    dealFlopCards5Timer->stop();
    dealFlopCards6Timer->stop();
    dealTurnCards0Timer->stop();
    dealTurnCards1Timer->stop();
    dealTurnCards2Timer->stop();
    dealRiverCards0Timer->stop();
    dealRiverCards1Timer->stop();
    dealRiverCards2Timer->stop();

    nextPlayerAnimationTimer->stop();
    preflopAnimation1Timer->stop();
    preflopAnimation2Timer->stop();
    flopAnimation1Timer->stop();
    flopAnimation2Timer->stop();
    turnAnimation1Timer->stop();
    turnAnimation2Timer->stop();
    riverAnimation1Timer->stop();
    riverAnimation2Timer->stop();

    postRiverAnimation1Timer->stop();
    postRiverRunAnimation1Timer->stop();
    postRiverRunAnimation2Timer->stop();
    postRiverRunAnimation3Timer->stop();
    postRiverRunAnimation5Timer->stop();
    postRiverRunAnimation6Timer->stop();
    potDistributeTimer->stop();
}

void GameTableWindow::setSpeeds()
{

    gameSpeed = (11 - guiGameSpeed) * 10;
    dealCardsSpeed = (gameSpeed / 2) * 10;    // milliseconds
    preDealCardsSpeed = dealCardsSpeed * 2;   // Zeit for Karten aufdecken auf dem Board (Flop, Turn, River)
    postDealCardsSpeed = dealCardsSpeed * 3;  // Zeit nach Karten aufdecken auf dem Board (Flop, Turn, River)
    AllInDealCardsSpeed = dealCardsSpeed * 4; // Zeit nach Karten aufdecken auf dem Board (Flop, Turn, River) bei AllIn
    postRiverRunAnimationSpeed = gameSpeed * 18;
    winnerBlinkSpeed = gameSpeed * 3; // milliseconds
    newRoundSpeed = gameSpeed * 35;
    nextPlayerSpeed1 = gameSpeed * 10; // Zeit zwischen dem Setzen des Spielers und dem Verdunkeln
    nextPlayerSpeed2 = gameSpeed * 4;  // Zeit zwischen Verdunkeln des einen und aufhellen des anderen Spielers
    nextPlayerSpeed3 = gameSpeed * 7;  // Zeit bis zwischen Aufhellen und Aktion
    preflopNextPlayerSpeed =
        gameSpeed * 10; // Zeit bis zwischen Aufhellen und Aktion im Preflop (etwas langsamer da nicht gerechnet wird. )
}

void GameTableWindow::breakButtonClicked()
{

    if (pushButton_break->text() == tr("Show cards"))
    {

        pushButton_break->setDisabled(true);
        breakAfterCurrentHand = true;
        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->setCardsShown(true);
    }
    else if (pushButton_break->text() == tr("Start"))
    {

        myStartWindow->getSession()->getCurrentGame()->getCurrentHand()->setCardsShown(false);

        blinkingStartButtonAnimationTimer->stop();
        // Set default Color
        myGameTableStyle->setBreakButtonStyle(pushButton_break, 0);
        QFontMetrics tempMetrics = this->fontMetrics();
        int width = tempMetrics.horizontalAdvance(tr("Show cards"));
        pushButton_break->setMinimumSize(width + 10, 20);

        pushButton_break->setText(tr("Show cards"));

        if (currentGameOver)
        {
            currentGameOver = false;
            myStartWindow->startNewGame();
        }
        else
        {
            startNewHand();
        }
    }
}

void GameTableWindow::changePlayingMode()
{

    int mode = -1;

    if (radioButton_manualAction->isChecked())
    {
        mode = 0;
    }
    if (radioButton_autoCheckFold->isChecked())
    {
        mode = 2;
    }
    if (radioButton_autoCheckCallAny->isChecked())
    {
        mode = 1;
    }

    playingMode = mode;
}

bool GameTableWindow::eventFilter(QObject* obj, QEvent* event)
{
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    if (/*obj == lineEdit_ChatInput && lineEdit_ChatInput->text() != "" && */ event->type() == QEvent::KeyPress &&
        keyEvent->key() == Qt::Key_Tab)
    {
        // myChat->nickAutoCompletition();
        return true;
        //    CURRENTLY not possible in Android :(  -->     } else if (event->type() == QEvent::KeyPress &&
        //    keyEvent->key() == Qt::Key_Back) {
        //            event->ignore();
        //            closeGameTable();
        //            return true;
    }
    else if (event->type() == QEvent::Close)
    {
        event->ignore();
        closeGameTable();
        return true;
    }
    else
    {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}

void GameTableWindow::switchLogWindow()
{

    int tab = 0;
    if (groupBox_RightToolBox->isHidden())
    {
        tabWidget_Right->setCurrentIndex(tab);
        groupBox_RightToolBox->show();
    }
    else
    {
        if (tabWidget_Right->currentIndex() == tab)
        {
            groupBox_RightToolBox->hide();
        }
        else
        {
            tabWidget_Right->setCurrentIndex(tab);
        }
    }
}

void GameTableWindow::switchAwayWindow()
{

    int tab = 1;
    if (groupBox_RightToolBox->isHidden())
    {
        tabWidget_Right->setCurrentIndex(tab);
        groupBox_RightToolBox->show();
    }
    else
    {
        if (tabWidget_Right->currentIndex() == tab)
        {
            groupBox_RightToolBox->hide();
        }
        else
        {
            tabWidget_Right->setCurrentIndex(tab);
        }
    }
}

void GameTableWindow::switchChanceWindow()
{

    int tab = 2;

    if (groupBox_RightToolBox->isHidden())
    {
        tabWidget_Right->setCurrentIndex(tab);
        groupBox_RightToolBox->show();
    }
    else
    {
        if (tabWidget_Right->currentIndex() == tab)
        {
            groupBox_RightToolBox->hide();
        }
        else
        {
            tabWidget_Right->setCurrentIndex(tab);
        }
    }
}

void GameTableWindow::switchFullscreen()
{

    if (this->isFullScreen())
    {
        this->showNormal();
    }
    else
    {
        this->showFullScreen();
    }
}

void GameTableWindow::blinkingStartButtonAnimationAction()
{

    QString style = pushButton_break->styleSheet();
    if (style.contains("QPushButton:enabled { background-color: #" + myGameTableStyle->getBreakLobbyButtonBgColor()))
    {
        myGameTableStyle->setBreakButtonStyle(pushButton_break, 1);
    }
    else
    {
        myGameTableStyle->setBreakButtonStyle(pushButton_break, 0);
    }
}

void GameTableWindow::GameModification()
{

    pushButton_break->show();

    QFontMetrics tempMetrics = this->fontMetrics();
    int width = tempMetrics.horizontalAdvance(tr("Show cards"));

    pushButton_break->setText(tr("Show cards"));
    pushButton_break->setMinimumSize(width + 10, 20);

    // Set the playing mode to "manual"
    radioButton_manualAction->click();
}

void GameTableWindow::updateMyButtonsState(int mode)
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    if (currentHand->getPreviousPlayerID() == 0)
    {
        myButtonsCheckable(false);
        clearMyButtons();
    }
    else
    {
        if (currentHand->getSeatsList()->front()->getAction() != PLAYER_ACTION_ALLIN)
        { // dont show pre-actions after flip cards when allin
            myButtonsCheckable(true);
            provideMyActions(mode);
        }
    }
}

void GameTableWindow::uncheckMyButtons()
{

    pushButton_BetRaise->setChecked(false);
    pushButton_CallCheck->setChecked(false);
    pushButton_Fold->setChecked(false);
    pushButton_AllIn->setChecked(false);
}

void GameTableWindow::resetButtonsCheckStateMemory()
{

    pushButtonCallCheckIsChecked = false;
    pushButtonFoldIsChecked = false;
    pushButtonAllInIsChecked = false;
    pushButtonBetRaiseIsChecked = false;
}

void GameTableWindow::clearMyButtons()
{

    pushButton_BetRaise->setText("");
    pushButton_CallCheck->setText("");
    pushButton_Fold->setText("");
    pushButton_AllIn->setText("");
}

void GameTableWindow::myButtonsCheckable(bool state)
{

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    if (state)
    {
        // checkable

        // exception: full bet rule
        if (!currentHand->getCurrentBettingRound()->getFullBetRule())
        {
            pushButton_BetRaise->setCheckable(true);
            pushButton_BetRaiseHalfPot->setCheckable(true);
            pushButton_BetRaiseTwoThirdPot->setCheckable(true);
            pushButton_BetRaisePot->setCheckable(true);
        }
        pushButton_CallCheck->setCheckable(true);
        pushButton_Fold->setCheckable(true);
        pushButton_AllIn->setCheckable(true);

        // design
        myGameTableStyle->setButtonsStyle(pushButton_BetRaise, pushButton_BetRaiseHalfPot,
                                          pushButton_BetRaiseTwoThirdPot, pushButton_BetRaisePot, pushButton_CallCheck,
                                          pushButton_Fold, pushButton_AllIn, 2);

        myButtonsAreCheckable = true;
    }
    else
    {
        // not checkable

        pushButton_BetRaise->setCheckable(false);
        pushButton_BetRaiseHalfPot->setCheckable(false);
        pushButton_BetRaiseTwoThirdPot->setCheckable(false);
        pushButton_BetRaisePot->setCheckable(false);
        pushButton_CallCheck->setCheckable(false);
        pushButton_Fold->setCheckable(false);
        pushButton_AllIn->setCheckable(false);

        QString hover;
        if (pushButton_AllIn->text() == AllInString)
        {
            myGameTableStyle->setButtonsStyle(pushButton_BetRaise, pushButton_BetRaiseHalfPot,
                                              pushButton_BetRaiseTwoThirdPot, pushButton_BetRaisePot,
                                              pushButton_CallCheck, pushButton_Fold, pushButton_AllIn, 0);
        }
        else
        {
            myGameTableStyle->setButtonsStyle(pushButton_BetRaise, pushButton_BetRaiseHalfPot,
                                              pushButton_BetRaiseTwoThirdPot, pushButton_BetRaisePot,
                                              pushButton_CallCheck, pushButton_Fold, pushButton_AllIn, 1);
        }

        myButtonsAreCheckable = false;
    }
}

void GameTableWindow::showMaximized()
{
    this->showFullScreen();
}

void GameTableWindow::closeGameTable()
{

    bool close = true;

    if (close)
    {
        stopTimer();
        myStartWindow->show();
        this->hide();
    }
}

void GameTableWindow::changeSpinBoxBetValue(int value)
{
    if (betSliderChangedByInput)
    {
        // prevent interval cutting of spinBox_betValue input from code below
        betSliderChangedByInput = false;
    }
    else
    {

        if (horizontalSlider_bet->value() == horizontalSlider_bet->maximum())
        {

            spinBox_betValue->setValue(horizontalSlider_bet->value());
        }
        else
        {

            int temp;
            if (horizontalSlider_bet->maximum() <= 1000)
            {
                temp = (int) ((value / 10) * 10);
            }
            else if (horizontalSlider_bet->maximum() > 1000 && horizontalSlider_bet->maximum() <= 10000)
            {
                temp = (int) ((value / 50) * 50);
            }
            else if (horizontalSlider_bet->maximum() > 10000 && horizontalSlider_bet->maximum() <= 100000)
            {
                temp = (int) ((value / 500) * 500);
            }
            else
            {
                temp = (int) ((value / 5000) * 5000);
            }

            if (temp < horizontalSlider_bet->minimum())
                spinBox_betValue->setValue(horizontalSlider_bet->minimum());
            else
                spinBox_betValue->setValue(temp);
        }
    }
}

void GameTableWindow::spinBoxBetValueChanged(int value)
{

    if (horizontalSlider_bet->isEnabled())
    {

        QString betRaise = pushButton_BetRaise->text().section("\n", 0, 0);

        if (value >= horizontalSlider_bet->minimum())
        {

            if (value > horizontalSlider_bet->maximum())
            { // print the maximum
                pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(horizontalSlider_bet->maximum()));
                betSliderChangedByInput = true;
                horizontalSlider_bet->setValue(horizontalSlider_bet->maximum());
            }
            else
            { // really print the value
                pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(value));
                betSliderChangedByInput = true;
                horizontalSlider_bet->setValue(value);
            }
        }
        else
        { // print the minimum
            pushButton_BetRaise->setText(betRaise + "\n$" + QString("%L1").arg(horizontalSlider_bet->minimum()));
            betSliderChangedByInput = true;
            horizontalSlider_bet->setValue(horizontalSlider_bet->minimum());
        }
    }
}

void GameTableWindow::refreshHandsRanges()
{
    const string style =
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; "
        "text-indent:0px;\"><span style=\" font-family:'Ubuntu';  font-size:8pt; font-weight:400; color:#000000;\">";
    stringstream displayText;

    static map<char, string> colors;
    colors['d'] = "<font size=+1 color=#FF0000><b><>&diams;</b></font>";
    colors['h'] = "<font size=+1 color=#FF0000><b>&hearts;</b></font>";
    colors['s'] = "<font size=+1><b>&spades;</b></font>";
    colors['c'] = "<font size=+1><b>&clubs;</b></font>";

    std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();

    PlayerList players = currentHand->getActivePlayerList();

    PlayerListConstIterator it_c;

    for (it_c = players->begin(); it_c != players->end(); ++it_c)
    {

        // display ranges for every player who didn't fold preflop
        if ((*it_c)->getCurrentHandActions().getPreflopActions().size() > 0 &&
            (*it_c)->getCurrentHandActions().getPreflopActions().front() != PLAYER_ACTION_FOLD)
        {

            displayText << style << "<b>" << ((*it_c)->getName() == " " ? "Human" : (*it_c)->getName()) << "</b> : ";

            std::istringstream oss((*it_c)->getEstimatedRange());
            std::string singleRange;

            while (getline(oss, singleRange, ','))
            {

                if (singleRange.size() != 4 || singleRange.find('+') != string::npos)
                {
                    displayText << "," << singleRange;
                    continue;
                }

                // display a real hand with graphical symbols for clubs, spade, heart and diamonds
                displayText << "," << singleRange.at(0) << colors[singleRange.at(1)] << singleRange.at(2)
                            << colors[singleRange.at(3)];
            }

            displayText << "<BR>";
        }
    }

    text_ranges->clear();
    text_ranges->append(displayText.str().c_str());
}

void GameTableWindow::refreshCardsChance(GameState BettingRound)
{

    std::shared_ptr<Player> player = myStartWindow->getSession()->getCurrentGame()->getSeatsList()->front();
    std::shared_ptr<HumanPlayer> humanPlayer = std::static_pointer_cast<HumanPlayer>(player);

    const string style =
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; "
        "text-indent:0px;\"><span style=\" font-family:'Ubuntu';  font-size:8pt; font-weight:400; color:#ffffff;\">";

    stringstream displayText;
    displayText.precision(0);

    if (humanPlayer->getActiveStatus())
    {

        std::shared_ptr<IHand> currentHand = myStartWindow->getSession()->getCurrentGame()->getCurrentHand();
        PlayerList players = currentHand->getActivePlayerList();

        if (humanPlayer->getAction() == PLAYER_ACTION_FOLD || currentHand->getCurrentRound() == GAME_STATE_PREFLOP)
        {
            label_chance->setText("");
        }
        else
        {

            string state = "Flop";
            if (currentHand->getCurrentRound() == GAME_STATE_TURN)
                state = "Turn";
            if (currentHand->getCurrentRound() == GAME_STATE_RIVER)
                state = "River";

            displayText << style << state << " :";

            const int nbOpponents = max(1, currentHand->getRunningPlayerList()->size() - 1);

            if (nbOpponents > 0)
            {

                SimResults r = humanPlayer->getHandSimulation();

                displayText << style << "<br>" << tr("Against ").toStdString() << nbOpponents
                            << tr(" opponent(s) with random hands :").toStdString();

                displayText << style << fixed << tr("I have ").toStdString() << r.win * 100
                            << tr("% chances to win right now<br>").toStdString() << tr("I have ").toStdString()
                            << r.winSd * 100 << tr("% chances to win at showdown<br>").toStdString()
                            << tr("I have ").toStdString() << r.tieSd * 100
                            << tr("% chances to tie at showdown").toStdString();
            }

            label_chance->setText(QApplication::translate("gametableimpl", displayText.str().c_str(), 0));
        }
    }
}

void GameTableWindow::refreshGameTableStyle()
{
    myGameTableStyle->setWindowsGeometry(this);
    myGameTableStyle->setChatLogStyle(textBrowser_Log);

    int i;
    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {

        myGameTableStyle->setCashLabelStyle(cashLabelArray[i]);
        myGameTableStyle->setSetLabelStyle(setLabelArray[i]);
        myGameTableStyle->setPlayerNameLabelStyle(playerNameLabelArray[i]);
    }

    myGameTableStyle->setSmallFontBoardStyle(label_Sets);
    myGameTableStyle->setSmallFontBoardStyle(label_Total);
    myGameTableStyle->setSmallFontBoardStyle(textLabel_Sets);
    myGameTableStyle->setSmallFontBoardStyle(textLabel_Pot);
    myGameTableStyle->setSmallFontBoardStyle(label_handNumber);
    myGameTableStyle->setSmallFontBoardStyle(label_gameNumber);
    myGameTableStyle->setSmallFontBoardStyle(label_handNumberValue);
    myGameTableStyle->setSmallFontBoardStyle(label_gameNumberValue);
    myGameTableStyle->setBigFontBoardStyle(textLabel_handLabel);
    myGameTableStyle->setBigFontBoardStyle(label_Pot);
    myGameTableStyle->setCardHolderStyle(label_CardHolder0, 0);
    myGameTableStyle->setCardHolderStyle(label_CardHolder1, 0);
    myGameTableStyle->setCardHolderStyle(label_CardHolder2, 0);
    myGameTableStyle->setCardHolderStyle(label_CardHolder3, 1);
    myGameTableStyle->setCardHolderStyle(label_CardHolder4, 2);
    myGameTableStyle->setTableBackground(this);
    myGameTableStyle->setBreakButtonStyle(pushButton_break, 0);

    for (i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {

        myGameTableStyle->setPlayerSeatInactiveStyle(groupBoxArray[i]);
    }
    // Human player button
    myGameTableStyle->setButtonsStyle(pushButton_BetRaise, pushButton_BetRaiseHalfPot, pushButton_BetRaiseTwoThirdPot,
                                      pushButton_BetRaisePot, pushButton_CallCheck, pushButton_Fold, pushButton_AllIn,
                                      0);

    myGameTableStyle->setBetValueInputStyle(spinBox_betValue);
    myGameTableStyle->setSliderStyle(horizontalSlider_bet);

    // 	away radiobuttons
    myGameTableStyle->setAwayRadioButtonsStyle(radioButton_manualAction);
    myGameTableStyle->setAwayRadioButtonsStyle(radioButton_autoCheckFold);
    myGameTableStyle->setAwayRadioButtonsStyle(radioButton_autoCheckCallAny);

    myGameTableStyle->setToolBoxBackground(groupBox_RightToolBox);

    myGameTableStyle->setTabWidgetStyle(tabWidget_Right, tabWidget_Right->getTabBar());

    if (myGameTableStyle->getActionCallI18NString() == "NULL")
    {
        CallString = "Call";
    }
    else
    {
        CallString = myGameTableStyle->getActionCallI18NString();
    }
    if (myGameTableStyle->getActionCheckI18NString() == "NULL")
    {
        CheckString = "Check";
    }
    else
    {
        CheckString = myGameTableStyle->getActionCheckI18NString();
    }
    if (myGameTableStyle->getActionBetI18NString() == "NULL")
    {
        BetString = "Bet";
    }
    else
    {
        BetString = myGameTableStyle->getActionBetI18NString();
    }
    if (myGameTableStyle->getActionRaiseI18NString() == "NULL")
    {
        RaiseString = "Raise";
    }
    else
    {
        RaiseString = myGameTableStyle->getActionRaiseI18NString();
    }
    if (myGameTableStyle->getActionFoldI18NString() == "NULL")
    {
        FoldString = "Fold";
    }
    else
    {
        FoldString = myGameTableStyle->getActionFoldI18NString();
    }
    if (myGameTableStyle->getActionAllInI18NString() == "NULL")
    {
        AllInString = "All-In";
    }
    else
    {
        AllInString = myGameTableStyle->getActionAllInI18NString();
    }
    if (myGameTableStyle->getPotI18NString() == "NULL")
    {
        PotString = "Pot";
    }
    else
    {
        PotString = myGameTableStyle->getPotI18NString();
    }
    if (myGameTableStyle->getTotalI18NString() == "NULL")
    {
        TotalString = "Total";
    }
    else
    {
        TotalString = myGameTableStyle->getTotalI18NString();
    }
    if (myGameTableStyle->getBetsI18NString() == "NULL")
    {
        BetsString = "Bets";
    }
    else
    {
        BetsString = myGameTableStyle->getBetsI18NString();
    }
    if (myGameTableStyle->getGameI18NString() == "NULL")
    {
        GameString = "Game";
    }
    else
    {
        GameString = myGameTableStyle->getGameI18NString();
    }
    if (myGameTableStyle->getHandI18NString() == "NULL")
    {
        HandString = "Hand";
    }
    else
    {
        HandString = myGameTableStyle->getHandI18NString();
    }
    if (myGameTableStyle->getPreflopI18NString() == "NULL")
    {
        PreflopString = "Preflop";
    }
    else
    {
        PreflopString = myGameTableStyle->getPreflopI18NString();
    }
    if (myGameTableStyle->getFlopI18NString() == "NULL")
    {
        FlopString = "Flop";
    }
    else
    {
        FlopString = myGameTableStyle->getFlopI18NString();
    }
    if (myGameTableStyle->getTurnI18NString() == "NULL")
    {
        TurnString = "Turn";
    }
    else
    {
        TurnString = myGameTableStyle->getTurnI18NString();
    }
    if (myGameTableStyle->getRiverI18NString() == "NULL")
    {
        RiverString = "River";
    }
    else
    {
        RiverString = myGameTableStyle->getRiverI18NString();
    }

    label_Pot->setText(PotString);
    label_Total->setText(TotalString + ":");
    label_Sets->setText(BetsString + ":");
    label_handNumber->setText(HandString + ":");
    label_gameNumber->setText(GameString + ":");
}

void GameTableWindow::closeMessageBoxes()
{
}

void GameTableWindow::hide()
{
    // clear log

    textBrowser_Log->clear();
    // textBrowser_ranges->clear();
    QWidget::hide();
}

SeatState GameTableWindow::getCurrentSeatState(std::shared_ptr<Player> player)
{

    if (player->getActiveStatus())
    {
        if (player->isSessionActive())
        {
            return SEAT_ACTIVE;
        }
        else
        {
            return SEAT_AUTOFOLD;
        }
    }
    else
    {
        return SEAT_CLEAR;
    }
    return SEAT_UNDEFINED;
}

void GameTableWindow::enableCallCheckPushButton()
{
    pushButton_CallCheck->setEatMyEvents(false);
}

void GameTableWindow::setGameSpeed(const int theValue)
{
    guiGameSpeed = theValue;
    setSpeeds();
}

GameTableStyleReader* GameTableWindow::getGameTableStyle() const
{
    return myGameTableStyle;
}

void GameTableWindow::setStartWindow(StartWindow* s)
{
    myStartWindow = s;
}
void GameTableWindow::setGuiDisplayGameActions(GuiDisplayGameActions* l)
{
    myGuiDisplayGameActions = l;
}
