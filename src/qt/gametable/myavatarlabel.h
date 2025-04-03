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
#ifndef MYAVATARLABEL_H
#define MYAVATARLABEL_H

#include <iostream>
#include <qt/startwindow/startwindowimpl.h>
#include <QtGui>
#include <QtCore>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class gameTableImpl;
class startWindowImpl;

class MyAvatarLabel : public QLabel
{
	Q_OBJECT
public:
	MyAvatarLabel(QGroupBox*);
	~MyAvatarLabel();

	void setW(gameTableImpl* theValue) {
		myW = theValue;
	}
	void setId ( int theValue ) {
		myId = theValue;
	}
	void contextMenuEvent ( QContextMenuEvent * event );
	QString getPlayerTip(QString);
	int getPlayerRating(QString);

public slots:

	void showContextMenu(const QPoint &pos);
	//void sendTriggerVoteOnKickSignal();
	void setEnabledContextMenu(bool);
	//void setVoteOnKickContextMenuEnabled(bool);
	//void setVoteRunning ( bool theValue ) {
	//	voteRunning = theValue;
	//}
	void setPixmap ( const QPixmap &, const bool = false);
	void setPixmapAndCountry ( const QPixmap &, QString country, int seatPlace, const bool = false);
	void setPixmapPath ( const QString theValue) {
		myPath = theValue;
	}
	void paintEvent(QPaintEvent*);
private:

	gameTableImpl *myW;
	QMenu *myContextMenu;
	//QAction *action_VoteForKick;
	//QAction *action_IgnorePlayer;
	//QAction *action_ReportBadAvatar;
	QAction *action_EditTip;

	QPixmap myPixmap;
	QString myPath;

	int myId;
	bool myContextMenuEnabled;
	//bool voteRunning;
	bool transparent;
};

#endif
