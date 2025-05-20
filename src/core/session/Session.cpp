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

#include "Session.h"
#include <core/engine/Game.h>
#include <core/engine/GameEvents.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/interfaces/IGui.h>

#include <core/engine/EngineFactory.h>
#include <core/engine/Randomizer.h>
#include <core/interfaces/ILogger.h>
#include <core/player/HumanPlayer.h>
#include <core/player/LooseAggressivePlayer.h>
#include <core/player/ManiacPlayer.h>
#include <core/player/TightAgressivePlayer.h>
#include <core/player/UltraTightPlayer.h>

#include <algorithm>
#include <random>
#include <sstream>

using namespace std;

Session::Session(GameEvents* events, ILogger* logger, IGui* g, IRankingStore* rs, IPlayersStatisticsStore* ps,
                 IHandAuditStore* ha)
    : myLogger(logger), currentGameNum(0), myGui(g), myRankingStore(rs), myPlayersStatisticsStore(ps),
      myHandAuditStore(ha), myEvents(events)
{
}

Session::~Session()
{
}

void Session::startGame(const GameData& gameData, const StartData& startData)
{

    currentGame.reset();
    currentGameNum++;

    if (myEvents && myEvents->onHideHoleCards)
        myEvents->onHideHoleCards();

    myGui->initGui(gameData.guiSpeed);

    std::shared_ptr<EngineFactory> factory(new EngineFactory(myEvents, myLogger));

    PlayerList playerList;
    playerList.reset(new std::list<std::shared_ptr<Player>>);

    TableProfile tableProfile = gameData.tableProfile;

    int nbLoose = 0;
    int nbManiac = 0;
    int nbTight = 0;
    int nbUltraTight = 0;
    int rand = 0;

    for (int i = 0; i < startData.numberOfPlayers; i++)
    {

        Player* player;

        if (i == 0)
        {
            player = new HumanPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i, PLAYER_TYPE_HUMAN,
                                     HumanPlayerName[0], gameData.startMoney, startData.numberOfPlayers > i,
                                     i == 0 ? true : false, 0);
        }
        else
        {

            if (tableProfile == TIGHT_AGRESSIVE_OPPONENTS)
            {

                Randomizer::GetRand(1, 3, 1, &rand);

                if (rand == 1)
                    player =
                        new TightAgressivePlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                 PLAYER_TYPE_COMPUTER, TightAgressivePlayerName[i], gameData.startMoney,
                                                 startData.numberOfPlayers > i, i == 0 ? true : false, 0);
                else
                    player = new UltraTightPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                  PLAYER_TYPE_COMPUTER, UltraTightPlayerName[i], gameData.startMoney,
                                                  startData.numberOfPlayers > i, i == 0 ? true : false, 0);
            }

            if (tableProfile == LARGE_AGRESSIVE_OPPONENTS)
            {

                player =
                    new LooseAggressivePlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                              PLAYER_TYPE_COMPUTER, LooseAggressivePlayerName[i], gameData.startMoney,
                                              startData.numberOfPlayers > i, i == 0 ? true : false, 0);
            }

            if (tableProfile == MANIAC_OPPONENTS)
            {

                player = new ManiacPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i, PLAYER_TYPE_COMPUTER,
                                          ManiacPlayerName[i], gameData.startMoney, startData.numberOfPlayers > i,
                                          i == 0 ? true : false, 0);
            }

            if (tableProfile == RANDOM_OPPONENTS)
            {

                Randomizer::GetRand(1, 12, 1, &rand);

                if (rand < 3 && nbManiac < startData.numberOfPlayers / 3)
                {
                    player = new ManiacPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                              PLAYER_TYPE_COMPUTER, ManiacPlayerName[i], gameData.startMoney,
                                              startData.numberOfPlayers > i, i == 0 ? true : false, 0);
                    nbManiac++;
                }
                else if (rand < 5 && nbUltraTight < startData.numberOfPlayers / 3)
                {
                    player = new UltraTightPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                  PLAYER_TYPE_COMPUTER, UltraTightPlayerName[i], gameData.startMoney,
                                                  startData.numberOfPlayers > i, i == 0 ? true : false, 0);
                    nbUltraTight++;
                }
                else if (rand < 9 && nbLoose < startData.numberOfPlayers / 3)
                {
                    player = new LooseAggressivePlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                       PLAYER_TYPE_COMPUTER, LooseAggressivePlayerName[i],
                                                       gameData.startMoney, startData.numberOfPlayers > i,
                                                       i == 0 ? true : false, 0);
                    nbLoose++;
                }
                else if (nbTight < startData.numberOfPlayers / 3)
                {
                    player =
                        new TightAgressivePlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                 PLAYER_TYPE_COMPUTER, TightAgressivePlayerName[i], gameData.startMoney,
                                                 startData.numberOfPlayers > i, i == 0 ? true : false, 0);
                    nbTight++;
                }
                else
                    // default
                    player = new UltraTightPlayer(myEvents, myHandAuditStore, myPlayersStatisticsStore, i,
                                                  PLAYER_TYPE_COMPUTER, UltraTightPlayerName[i], gameData.startMoney,
                                                  startData.numberOfPlayers > i, i == 0 ? true : false, 0);
            }

            if (i > startData.numberOfPlayers / 2)
                tableProfile = RANDOM_OPPONENTS; // fill the remaining opponents with random profiles */
        }

        player->setIsSessionActive(true);
        playerList->push_back(std::shared_ptr<Player>(player));
    }

    // random shuffle the list (only the computer opponents, not the human):

    vector<std::shared_ptr<Player>> v(playerList->size());
    copy(++(playerList->begin()), playerList->end(), ++(v.begin()));
    std::mt19937 rng(std::time(nullptr));
    std::shuffle(++(v.begin()), v.end(), rng);
    (*v.begin()) = (*playerList->begin()); // the human player

    playerList->clear();

    int id = 0;
    for (vector<std::shared_ptr<Player>>::iterator i = v.begin(); i != v.end(); i++)
    {
        (*i)->setID(id++);
        playerList->push_back(*i);
    }

    currentGame.reset(new Game(myGui, factory, playerList, gameData, startData, currentGameNum, myRankingStore,
                               myPlayersStatisticsStore, myHandAuditStore));
    currentGame->initHand();
    currentGame->startHand();
}

std::shared_ptr<Game> Session::getCurrentGame()
{
    return currentGame;
}

IGui* Session::getGui()
{
    return myGui;
}
