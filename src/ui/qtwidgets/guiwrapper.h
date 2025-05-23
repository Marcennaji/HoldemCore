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

#include "core/engine/model/GameData.h"
#include "ui/interfaces/IGui.h"

#include <string>

class Session;
class GameTableWindow;
class StartWindow;
class Game;

class GuiWrapper : public IGui
{
  public:
    GuiWrapper(const std::string& appDataDir, StartWindow*);

    ~GuiWrapper();

    void setStartWindow(void* w);
    void initGui(int speed);

    void* getGameTableWindow() const override { return static_cast<void*>(myW); }

    void dealBettingRoundCards(int myBettingRoundID);
    void dealHoleCards();

    void preflopAnimation2();

    void flopAnimation1();
    void flopAnimation2();

    void turnAnimation2();

    void riverAnimation1();
    void riverAnimation2();

    void postRiverAnimation1();
    void postRiverRunAnimation1();

  private:
    GameTableWindow* myW;
    StartWindow* myStartWindow;
};

#endif
