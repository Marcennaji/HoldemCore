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
#ifndef MYCASHLABEL_H
#define MYCASHLABEL_H

#include <iostream>

#include <QtGui>
#include <QtCore>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class gameTableImpl;

class MyCashLabel : public QLabel
{
	Q_OBJECT
public:
	MyCashLabel(QGroupBox*);
	~MyCashLabel();

	void setW(gameTableImpl* theValue) {
		myW = theValue;
	}

public slots:

	void setText ( const QString &, bool = false );
private:

	QString myText;
	gameTableImpl *myW;

};

#endif
