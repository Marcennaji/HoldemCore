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
#include "GameTableWindow.h"
#include "mysetlabel.h"

using namespace std;

MyStatusLabel::MyStatusLabel(QGroupBox* parent) : QLabel(parent), myW(NULL), mousePress(false)
{

    mousePress = false;
}

MyStatusLabel::~MyStatusLabel()
{
}

void MyStatusLabel::mousePressEvent(QMouseEvent* event)
{

    if (!mousePress && objectName().contains("textLabel_Status0"))
    {
        mousePress = true;
    }

    QLabel::mousePressEvent(event);
}

void MyStatusLabel::mouseReleaseEvent(QMouseEvent* event)
{

    if (mousePress && objectName().contains("textLabel_Status0"))
    {
        mousePress = false;
    }

    QLabel::mouseReleaseEvent(event);
}
