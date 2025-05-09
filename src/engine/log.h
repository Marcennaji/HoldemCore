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

#ifndef LOG_H
#define LOG_H

#define SQLLITE_COMPLETE_LOG

#include <string>
#include <filesystem>

#include "engine.h"
#include "game_defs.h"
#include "PlayerStatistics.h"

#define SQL_LOG_FILE "pokerTraining-log-v0.9.pdb"

struct sqlite3;

class Log
{

public:
	Log(const std::string &logDir);

	~Log();

	void init();
	void logGameLosers(PlayerList activePlayerList);
	void logGameWinner(PlayerList activePlayerList);
	void logPlayedGames(PlayerList activePlayerList);
	void logUnplausibleHand(const std::string card1, const std::string card2, const bool human, 
				const char bettingRound, const int nbPlayers);
	void logPlayersStatistics(PlayerList activePlayerList);
	void InitializePlayersStatistics(const std::string playerName, const int nbPlayers);
	std::array<PlayerStatistics, MAX_NUMBER_OF_PLAYERS + 1> getPlayerStatistics(const std::string & playerName);
	void createDatabase();

	void setCurrentRound(GameState theValue) {
		currentRound = theValue;
	}

	std::string getSqliteLogFileName() {
		return mySqliteLogFileName.string();
	}

private:

	void exec_transaction();
	int getIntegerValue(const std::string playerName, const std::string tableName, const std::string attributeName);
	void createRankingTable();
	void createUnplausibleHandsTable();

	sqlite3 *mySqliteLogDb;
	std::filesystem::path mySqliteLogFileName;
	int uniqueGameID;
	int currentHandID;
	GameState currentRound;
	std::string sql;
	std::string myLogDir;
};

#endif // LOG_H
