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

#ifndef ENGINEFACTORY_H
#define ENGINEFACTORY_H

#include <engine/enginefactory.h>

#include <engine/handinterface.h>
#include <engine/boardinterface.h>
#include <engine/Player.h>

#include <memory>
#include <vector>

class ConfigFile;

class EngineFactory 
{
public:
	EngineFactory(ConfigFile*);
	~EngineFactory();

	virtual std::shared_ptr<HandInterface> createHand(std::shared_ptr<EngineFactory> f, GuiInterface *g, std::shared_ptr<BoardInterface> b, Log *l, PlayerList sl, PlayerList apl, PlayerList rpl, int id, int sP, int dP, int sB,int sC);
	virtual std::shared_ptr<BoardInterface> createBoard(unsigned dp);
	//virtual std::shared_ptr<Player> createPlayer(int id, unsigned uniqueId, PlayerType type, std::string name, std::string avatar, int sC, bool aS, bool sotS, int mB);
	virtual std::vector<std::shared_ptr<BettingRoundInterface> > createBettingRound(HandInterface *hi, unsigned dP, int sB);

private:
	ConfigFile *myConfig;
};

#endif
