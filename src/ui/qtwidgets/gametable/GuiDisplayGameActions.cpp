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
#include "GuiDisplayGameActions.h"

#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/styles/gametablestylereader.h>

#include <core/engine/CardsValue.h>
#include <core/engine/Game.h>
#include <core/interfaces/IHand.h>

using namespace std;

GuiDisplayGameActions::GuiDisplayGameActions(GameTableWindow* w) : myW(w)
{

    myStyle = myW->getGameTableStyle();

    connect(this, SIGNAL(signalLogPlayerActionMsg(QString, int, int)), this,
            SLOT(logPlayerActionMsg(QString, int, int)));
    connect(this, SIGNAL(signalLogNewGameHandMsg(int, int)), this, SLOT(logNewGameHandMsg(int, int)));
    connect(this, SIGNAL(signalLogNewBlindsSetsMsg(int, int, QString, QString)), this,
            SLOT(logNewBlindsSetsMsg(int, int, QString, QString)));
    connect(this, SIGNAL(signalLogPlayerWinsMsg(QString, int, bool)), this, SLOT(logPlayerWinsMsg(QString, int, bool)));
    connect(this, SIGNAL(signalLogDealBoardCardsMsg(int, int, int, int, int, int)), this,
            SLOT(logDealBoardCardsMsg(int, int, int, int, int, int)));
    connect(this, SIGNAL(signalLogFlipHoleCardsMsg(QString, int, int, int, QString)), this,
            SLOT(logFlipHoleCardsMsg(QString, int, int, int, QString)));
}

GuiDisplayGameActions::~GuiDisplayGameActions()
{
}

void GuiDisplayGameActions::logPlayerActionMsg(QString msg, int action, int setValue)
{

    switch (action)
    {

    case 1:
        msg += " folds.";
        break;
    case 2:
        msg += " checks.";
        break;
    case 3:
        msg += " calls ";
        break;
    case 4:
        msg += " bets ";
        break;
    case 5:
        msg += " bets ";
        break;
    case 6:
        msg += " is all in with ";
        break;
    default:
        msg += "ERROR";
    }

    if (action >= 3)
    {
        msg += "$" + QString::number(setValue, 10) + ".";
    }

    myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() + ";\">" + msg + "</span>");
}

void GuiDisplayGameActions::logNewGameHandMsg(int gameID, int handID)
{

    PlayerListConstIterator it_c;
    std::shared_ptr<IHand> currentHand = myW->getSession()->getCurrentGame()->getCurrentHand();

    PlayerList activePlayerList = currentHand->getActivePlayerList();

    myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() +
                                 "; font-size:large; font-weight:bold\">## Game: " + QString::number(gameID, 10) +
                                 " | Hand: " + QString::number(handID, 10) + " ##</span>");
}

void GuiDisplayGameActions::logNewBlindsSetsMsg(int sbSet, int bbSet, QString sbName, QString bbName)
{

    // log blinds
    myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() + ";\">" + sbName +
                                 " posts small blind ($" + QString::number(sbSet, 10) + ")</span>");
    myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() + ";\">" + bbName +
                                 " posts big blind ($" + QString::number(bbSet, 10) + ")</span>");
}

void GuiDisplayGameActions::logPlayerWinsMsg(QString playerName, int pot, bool main)
{

    if (main)
    {
        myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getLogWinnerMainPotColor() + ";\">" +
                                     playerName + " wins $" + QString::number(pot, 10) + "</span>");
    }
    else
    {
        myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getLogWinnerSidePotColor() + ";\">" +
                                     playerName + " wins $" + QString::number(pot, 10) + " (side pot)</span>");
    }
}

void GuiDisplayGameActions::logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5)
{

    QString round;

    switch (roundID)
    {

    case 1:
        round = "Flop";
        myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getLogPlayerSitsOutColor() + ";\">--- " +
                                     round + " --- " + "[" + translateCardCode(card1).at(0) +
                                     translateCardCode(card1).at(1) + "," + translateCardCode(card2).at(0) +
                                     translateCardCode(card2).at(1) + "," + translateCardCode(card3).at(0) +
                                     translateCardCode(card3).at(1) + "]</span>");
        break;
    case 2:
        round = "Turn";
        myW->textBrowser_Log->append(
            "<span style=\"color:#" + myStyle->getLogPlayerSitsOutColor() + ";\">--- " + round + " --- " + "[" +
            translateCardCode(card1).at(0) + translateCardCode(card1).at(1) + "," + translateCardCode(card2).at(0) +
            translateCardCode(card2).at(1) + "," + translateCardCode(card3).at(0) + translateCardCode(card3).at(1) +
            "," + translateCardCode(card4).at(0) + translateCardCode(card4).at(1) + "]</span>");
        break;
    case 3:
        round = "River";
        myW->textBrowser_Log->append(
            "<span style=\"color:#" + myStyle->getLogPlayerSitsOutColor() + ";\">--- " + round + " --- " + "[" +
            translateCardCode(card1).at(0) + translateCardCode(card1).at(1) + "," + translateCardCode(card2).at(0) +
            translateCardCode(card2).at(1) + "," + translateCardCode(card3).at(0) + translateCardCode(card3).at(1) +
            "," + translateCardCode(card4).at(0) + translateCardCode(card4).at(1) + "," +
            translateCardCode(card5).at(0) + translateCardCode(card5).at(1) + "]</span>");
        break;
    default:
        round = "ERROR";
    }
}

void GuiDisplayGameActions::logFlipHoleCardsMsg(QString playerName, int card1, int card2, int cardsValueInt,
                                                QString showHas)
{

    QString tempHandName;

    if (cardsValueInt != -1)
    {

        tempHandName =
            CardsValue::determineHandName(cardsValueInt, myW->getSession()->getCurrentGame()->getActivePlayerList())
                .c_str();
        myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() + ";\">" + playerName +
                                     " " + showHas + " [" + translateCardCode(card1).at(0) +
                                     translateCardCode(card1).at(1) + "," + translateCardCode(card2).at(0) +
                                     translateCardCode(card2).at(1) + "] - \"" + tempHandName + "\"</span>");
    }
    else
    {
        myW->textBrowser_Log->append("<span style=\"color:#" + myStyle->getChatLogTextColor() + ";\">" + playerName +
                                     " " + showHas + " [" + translateCardCode(card1).at(0) +
                                     translateCardCode(card1).at(1) + "," + translateCardCode(card2).at(0) +
                                     translateCardCode(card2).at(1) + "]</span>");
    }
}

void GuiDisplayGameActions::logPlayerWinGame(QString playerName, int gameID)
{

    myW->textBrowser_Log->append("<i><b>" + playerName + " wins game " + QString::number(gameID, 10) + "!</i></b><br>");
}

QStringList GuiDisplayGameActions::translateCardCode(int cardCode)
{

    int value = cardCode % 13;
    int color = cardCode / 13;

    QStringList cardString;

    switch (value)
    {

    case 0:
        cardString << "2";
        break;
    case 1:
        cardString << "3";
        break;
    case 2:
        cardString << "4";
        break;
    case 3:
        cardString << "5";
        break;
    case 4:
        cardString << "6";
        break;
    case 5:
        cardString << "7";
        break;
    case 6:
        cardString << "8";
        break;
    case 7:
        cardString << "9";
        break;
    case 8:
        cardString << "T";
        break;
    case 9:
        cardString << "J";
        break;
    case 10:
        cardString << "Q";
        break;
    case 11:
        cardString << "K";
        break;
    case 12:
        cardString << "A";
        break;
    default:
        cardString << "ERROR";
    }

    switch (color)
    {

    case 0:
        cardString << "<font size=+1><b>&diams;</b></font>";
        break;
    case 1:
        cardString << "<font size=+1><b>&hearts;</b></font>";
        break;
    case 2:
        cardString << "<font size=+1><b>&spades;</b></font>";
        break;
    case 3:
        cardString << "<font size=+1><b>&clubs;</b></font>";
        break;
    default:
        cardString << "ERROR";
    }

    return cardString;
}

int GuiDisplayGameActions::convertCardStringToInt(string val, string col)
{

    int tmp;

    switch (*col.c_str())
    {
    case 'd':
        tmp = 0;
        break;
    case 'h':
        tmp = 13;
        break;
    case 's':
        tmp = 26;
        break;
    case 'c':
        tmp = 39;
        break;
    default:
        return -1;
    }

    switch (*val.c_str())
    {
    case '2':
        tmp += 0;
        break;
    case '3':
        tmp += 1;
        break;
    case '4':
        tmp += 2;
        break;
    case '5':
        tmp += 3;
        break;
    case '6':
        tmp += 4;
        break;
    case '7':
        tmp += 5;
        break;
    case '8':
        tmp += 6;
        break;
    case '9':
        tmp += 7;
        break;
    case 'T':
        tmp += 8;
        break;
    case 'J':
        tmp += 9;
        break;
    case 'Q':
        tmp += 10;
        break;
    case 'K':
        tmp += 11;
        break;
    case 'A':
        tmp += 12;
        break;
    default:
        return -1;
    }

    return tmp;
}

string GuiDisplayGameActions::convertCardIntToString(int code, int modus)
{

    string tmp;

    switch (code % 13)
    {
    case 0:
        tmp = "2";
        break;
    case 1:
        tmp = "3";
        break;
    case 2:
        tmp = "4";
        break;
    case 3:
        tmp = "5";
        break;
    case 4:
        tmp = "6";
        break;
    case 5:
        tmp = "7";
        break;
    case 6:
        tmp = "8";
        break;
    case 7:
        tmp = "9";
        break;
    case 8:
        tmp = "T";
        break;
    case 9:
        tmp = "J";
        break;
    case 10:
        tmp = "Q";
        break;
    case 11:
        tmp = "K";
        break;
    case 12:
        tmp = "A";
        break;
    default:
        return "";
    }

    if (modus == 2)
    {

        switch (code / 13)
        {
        case 0:
            tmp += "d";
            break;
        case 1:
            tmp += "h";
            break;
        case 2:
            tmp += "s";
            break;
        case 3:
            tmp += "c";
            break;
        default:
            return "";
        }
    }
    else
    {

        switch (code / 13)
        {
        case 0:
            tmp += "<font size=+1><b>&diams;</b></font>";
            break;
        case 1:
            tmp += "<font size=+1><b>&hearts;</b></font>";
            break;
        case 2:
            tmp += "<font size=+1><b>&spades;</b></font>";
            break;
        case 3:
            tmp += "<font size=+1><b>&clubs;</b></font>";
            break;
        default:
            return "";
        }
    }

    return tmp;
}
