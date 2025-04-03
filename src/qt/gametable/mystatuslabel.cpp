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
#include "mysetlabel.h"
#include "gametableimpl.h"

using namespace std;

MyStatusLabel::MyStatusLabel(QGroupBox* parent)
	: QLabel(parent), myW(NULL), mousePress(false)
{

	mousePress = false;

}

MyStatusLabel::~MyStatusLabel()
{
}

void MyStatusLabel::mousePressEvent(QMouseEvent * event)
{

	if (!mousePress && objectName().contains("textLabel_Status0")) {
		mousePress = true;
		myW->mouseOverFlipCards(true);

	}

	QLabel::mousePressEvent(event);
}

void MyStatusLabel::mouseReleaseEvent(QMouseEvent * event)
{

	if (mousePress && objectName().contains("textLabel_Status0")) {
		mousePress = false;
		myW->mouseOverFlipCards(false);
	}

	QLabel::mouseReleaseEvent(event);
}
