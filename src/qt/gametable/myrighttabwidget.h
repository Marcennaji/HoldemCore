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
#ifndef MYRIGHTTABWIDGET_H
#define MYRIGHTTABWIDGET_H

#include <QtGui>
#include <QtCore>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QGroupBox>

class MyRightTabWidget : public QTabWidget
{
	Q_OBJECT
public:
    MyRightTabWidget(QGroupBox*);

	~MyRightTabWidget();

	void paintEvent(QPaintEvent * event);

	QTabBar* getTabBar() const {
		return myTabBar;
	}


public slots:


private:
	QTabBar *myTabBar;

};

#endif
