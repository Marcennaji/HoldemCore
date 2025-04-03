/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
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

#include "enginefactory.h"

#include "hand.h"
#include "board.h"
#include "BettingRoundpreflop.h"
#include "BettingRoundflop.h"
#include "BettingRoundturn.h"
#include "BettingRoundriver.h"
#include "BettingRoundpostriver.h"
#include "tools.h"
#include "TightAgressivePlayer.h"
#include "LooseAggressivePlayer.h"
#include "ManiacPlayer.h"
#include "HumanPlayer.h"

#include <configfile.h>

EngineFactory::EngineFactory(ConfigFile *c)
	: myConfig(c)
{
}


EngineFactory::~EngineFactory()
{
}


std::shared_ptr<HandInterface>
EngineFactory::createHand(std::shared_ptr<EngineFactory> f, GuiInterface *g, std::shared_ptr<BoardInterface> b, Log *l, PlayerList sl, PlayerList apl, PlayerList rpl, int id, int sP, int dP, int sB,int sC)
{
	return std::shared_ptr<HandInterface>(new Hand(f, g, b, l, sl, apl, rpl, id, sP, dP, sB, sC));
}

std::shared_ptr<BoardInterface>
EngineFactory::createBoard(unsigned dp)
{
	return std::shared_ptr<BoardInterface>(new Board(dp));
}

//std::shared_ptr<Player>
//EngineFactory::createPlayer(int id, unsigned uniqueId, PlayerType type, 
//								std::string name, std::string avatar, 
//								int sC, bool aS, bool sotS, int mB)
//{
//	Player * player;
//
//	if (type == PLAYER_TYPE_HUMAN){
//		player = new HumanPlayer(myConfig, id, type, HumanPlayerName[0], avatar, sC, aS, sotS, mB);
//	}else{
//
//		int rand = 0;
//		Tools::GetRand(1, 3, 1, &rand);
//
//		if (rand == 1)
//			player = new ManiacPlayer(myConfig, id, type, ManiacPlayerName[id], avatar, sC, aS, sotS, mB);
//		else
//		if (rand == 2)
//			player = new LooseAggressivePlayer(myConfig, id, type, LooseAggressivePlayerName[id], avatar, sC, aS, sotS, mB);
//		else
//			player = new TightAgressivePlayer(myConfig, id, type, TightAgressivePlayerName[id], avatar, sC, aS, sotS, mB);
//	}
//
//	std::string avatarFile = myConfig->readConfigString("AppDataDir");
//	avatarFile += "/gfx/avatars/";
//	avatarFile += player->getName();
//	avatarFile += ".png";
//	player->setAvatar(avatarFile);
//
//	return std::shared_ptr<Player> (player);
//}

std::vector<std::shared_ptr<BettingRoundInterface> >
EngineFactory::createBettingRound(HandInterface *hi, unsigned dP, int sB)
{
	std::vector<std::shared_ptr<BettingRoundInterface> > myBettingRound;

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundPreflop(hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundFlop(hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundTurn(hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundRiver(hi, dP, sB)));

	myBettingRound.push_back(std::shared_ptr<BettingRoundInterface>(new BettingRoundPostRiver(hi, dP, sB)));

	return myBettingRound;

}
