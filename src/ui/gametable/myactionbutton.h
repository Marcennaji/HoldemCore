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
#ifndef MYACTIONBUTTON_H
#define MYACTIONBUTTON_H

#include <QtGui>
#include <QtCore>
#include <QtWidgets/QPushButton>

class GameTableStyleReader;

class MyActionButton : public QPushButton
{
public:
	MyActionButton(QWidget*);

	~MyActionButton();

	void paintEvent(QPaintEvent * event);
	void setFKeyText ( const QString& theValue );
	void setStyle ( GameTableStyleReader* theValue );
	void setEatMyEvents(bool e);
	bool event(QEvent *e);

private:

	QString fKeyText;
	GameTableStyleReader *myStyle;
	bool eatMyEvents;
};

#endif
