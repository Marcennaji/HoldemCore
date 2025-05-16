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
#ifndef GUILOG_H
#define GUILOG_H

#include <QObject>
#include <QStringList>

class GameTableWindow;
class GameTableStyleReader;

class GuiDisplayGameActions : public QObject
{
    Q_OBJECT

  public:
    GuiDisplayGameActions(GameTableWindow*);

    ~GuiDisplayGameActions();

  public slots:

    void logPlayerActionMsg(QString playerName, int action, int setValue);
    void logNewGameHandMsg(int gameID, int handID);
    void logNewBlindsSetsMsg(int sbSet, int bbSet, QString sbName, QString bbName);
    void logPlayerWinsMsg(QString playerName, int pot, bool main);
    void logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4 = -1, int card5 = -1);
    void logFlipHoleCardsMsg(QString playerName, int card1, int card2, int cardsValueInt = -1,
                             QString showHas = "shows");
    void logPlayerWinGame(QString playerName, int gameID);

  signals:
    void signalLogPlayerActionMsg(QString playerName, int action, int setValue);
    void signalLogNewGameHandMsg(int gameID, int handID);
    void signalLogNewBlindsSetsMsg(int sbSet, int bbSet, QString sbName, QString bbName);
    void signalLogPlayerWinsMsg(QString playerName, int pot, bool main);
    void signalLogDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4 = -1, int card5 = -1);
    void signalLogFlipHoleCardsMsg(QString playerName, int card1, int card2, int cardsValueInt = -1,
                                   QString showHas = "shows");
    void signalLogPlayerWinGame(QString playerName, int gameID);

  private:
    int convertCardStringToInt(std::string val, std::string col);
    std::string convertCardIntToString(int code, int modus);
    QStringList translateCardCode(int cardCode);

    GameTableWindow* myW;
    GameTableStyleReader* myStyle;

    friend class GuiWrapper;
};

#endif
