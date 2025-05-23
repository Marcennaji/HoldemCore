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
#include "guiwrapper.h"
#include <core/engine/Game.h>
#include <core/player/Player.h>
#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>
#include <ui/qtwidgets/gametable/GameTableWindow.h>
#include <ui/qtwidgets/startwindow/StartWindow.h>

using namespace std;

GuiWrapper::GuiWrapper(const std::string& appDataDir, StartWindow* s) : myW(NULL), myStartWindow(s)
{

    myW = new GameTableWindow(appDataDir);
}

GuiWrapper::~GuiWrapper()
{
}
void GuiWrapper::setStartWindow(void* w)
{
    myStartWindow = static_cast<StartWindow*>(w);

    if (myW)
        myW->setStartWindow(myStartWindow);
}

void GuiWrapper::initGui(int speed)
{
    myW->signalInitGui(speed);
}

void GuiWrapper::preflopAnimation2()
{
    myW->signalPreflopAnimation2();
}

void GuiWrapper::flopAnimation2()
{
    myW->signalFlopAnimation2();
}

void GuiWrapper::turnAnimation2()
{
    myW->signalTurnAnimation2();
}

void GuiWrapper::riverAnimation2()
{
    myW->signalRiverAnimation2();
}

void GuiWrapper::postRiverRunAnimation1()
{
    myW->signalPostRiverRunAnimation1();
}
