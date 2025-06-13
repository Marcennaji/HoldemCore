/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "mynamelabel.h"
#include <core/engine/Game.h>
#include <core/player/Player.h>
#include <core/session/Session.h>
#include <ui/qtwidgets/styles/gametablestylereader.h>
#include "GameTableWindow.h"

MyNameLabel::MyNameLabel(QGroupBox* parent) : QLabel(parent), myW(0)
{
}

MyNameLabel::~MyNameLabel()
{
}

void MyNameLabel::setText(const QString& t)
{
    QLabel::setText(t);
}
