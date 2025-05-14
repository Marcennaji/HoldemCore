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
#ifndef STDSESSION_H
#define STDSESSION_H

#include <memory>
#include <string>

#include <core/engine/GameData.h>
#include <core/engine/EngineDefs.h>
#include <core/engine/GameInfo.h>
#include <core/engine/StartData.h>
#include <core/engine/GameData.h>
#include <core/interfaces/persistence/ILogger.h>

class GuiInterface;
class Game;
class Log;
class QtToolsInterface;

class Session
{
public:

	Session(ILogger *, GuiInterface*, Log*);

	~Session();

	bool init();

	void startGame(const GameData &gameData, const StartData &startData);

	std::shared_ptr<Game> getCurrentGame();

	GuiInterface *getGui();

	Log* getLog() {
		return myLog;
	}


	GameInfo getClientGameInfo(unsigned gameId) const;
	unsigned getGameIdOfPlayer(unsigned playerId) const;
	unsigned getClientCurrentGameId() const;
	unsigned getClientUniquePlayerId() const;

private:

	int currentGameNum;

	std::shared_ptr<Game> currentGame;
	GuiInterface *myGui;
	ILogger * myLogger;
	Log *myLog;
	QtToolsInterface *myQtToolsInterface;
};

#endif
