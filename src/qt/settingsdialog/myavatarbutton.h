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
#ifndef MYAVATARBUTTON_H
#define MYAVATARBUTTON_H

#include <QtGui>
#include <QtCore>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>

class MyAvatarButton : public QPushButton
{
public:
	MyAvatarButton(QGroupBox*);

	~MyAvatarButton();

	void setLink(QString theValue) {
		myLink = theValue;
	}
	QString getLink() const {
		return myLink;
	}



private:

	QString myLink;
};

#endif
