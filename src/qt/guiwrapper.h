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
#ifndef GUIWRAPPER_H
#define GUIWRAPPER_H

#include <qt/guiinterface.h>
#include <gamedata.h>
#include <string>

class Session;
class gameTableImpl;
class startWindowImpl;
class guiLog;
class ConfigFile;
class Game;


class GuiWrapper : public GuiInterface
{
public:
	GuiWrapper(ConfigFile*, startWindowImpl*);

	~GuiWrapper();

	void initGui(int speed);

	std::shared_ptr<Session> getSession();
	void setSession(std::shared_ptr<Session> session);

	gameTableImpl* getW() const {
		return myW;
	}
	guiLog* getGuiLog() const {
		return myGuiLog;
	}

	void refreshSet() const;
	void refreshCash() const;
	void refreshAction(int =-1, int =-1) const;
	void refreshChangePlayer() const;
	void refreshPot() const;
	void refreshGroupbox(int =-1, int =-1) const;
	void refreshAll() const;
	void refreshPlayerName() const;
	void refreshButton() const;
	void refreshGameLabels(GameState state) const;

	void setPlayerAvatar(int myID, const std::string &myAvatar) const;

	void waitForGuiUpdateDone() const;

	void dealBettingRoundCards(int myBettingRoundID);
	void dealHoleCards();
	void dealFlopCards();
	void dealTurnCard();
	void dealRiverCard();

	void nextPlayerAnimation();

	void beRoAnimation2(int);

	void preflopAnimation1();
	void preflopAnimation2();

	void flopAnimation1();
	void flopAnimation2();

	void turnAnimation1();
	void turnAnimation2();

	void riverAnimation1();
	void riverAnimation2();

	void postRiverAnimation1();
	void postRiverRunAnimation1();

	void flipHolecardsAllIn();

	void nextRoundCleanGui();

	void meInAction();
	void showCards(unsigned playerId);
	void disableMyButtons();
	void updateMyButtonsState();
	void startTimeoutAnimation(int playerNum, int timeoutSec);
	void stopTimeoutAnimation(int playerNum);
	void logPlayerActionMsg(std::string playerName, int action, int setValue) ;
	void logNewGameHandMsg(int gameID, int handID) ;
	void logNewBlindsSetsMsg(int sbSet, int bbSet, std::string sbName, std::string bbName);
	void logPlayerWinsMsg(std::string playerName, int pot, bool main);
	void logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4 = -1, int card5 = -1);
	void logFlipHoleCardsMsg(std::string playerName, int card1, int card2, int cardsValueInt = -1, std::string showHas = "shows");
	void logPlayerWinGame(std::string playerName, int gameID);
	void flushLogAtGame(int gameID);
	void flushLogAtHand();
	void hideHoleCards();


private:

	guiLog *myGuiLog;
	gameTableImpl *myW;
	ConfigFile *myConfig;
	startWindowImpl *myStartWindow;

};

#endif
