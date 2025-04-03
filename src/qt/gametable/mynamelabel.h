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
#ifndef MYNAMELABEL_H
#define MYNAMELABEL_H

#include <iostream>

#include <QtGui>
#include <QtCore>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>

class gameTableImpl;

class MyNameLabel : public QLabel
{
	Q_OBJECT
public:
	MyNameLabel(QGroupBox*);
	~MyNameLabel();

	void setW(gameTableImpl* theValue) {
		myW = theValue;
	}

public slots:

	void setText ( const QString &, bool = false, bool = false, bool = false);
private:

	QString myText;
	gameTableImpl *myW;

};

#endif
