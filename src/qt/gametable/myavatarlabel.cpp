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
#include "myavatarlabel.h"

#include <qt/gametable/gametableimpl.h>
#include <qt/styles/gametablestylereader.h>
#include <session.h>
#include <engine/Player.h>
#include <engine/game.h>
#include <qt/mymessagedialog/mymessagedialogimpl.h>
#include <QSysInfo>

using namespace std;

MyAvatarLabel::MyAvatarLabel(QGroupBox* parent)
	: QLabel(parent), transparent(false)
{

	//myContextMenu = new QMenu;
	//action_EditTip = new QAction(QIcon(":/gfx/user_properties.png"), tr("Add/Edit/Remove tooltip"), myContextMenu);
	//myContextMenu->addAction(action_EditTip);
	//action_VoteForKick = new QAction(QIcon(":/gfx/list_remove_user.png"), tr("Start vote to kick this player"), myContextMenu);
	//myContextMenu->addAction(action_VoteForKick);
	//action_IgnorePlayer = new QAction(QIcon(":/gfx/im-ban-user.png"), tr("Ignore Player"), myContextMenu);
	//myContextMenu->addAction(action_IgnorePlayer);
	//action_ReportBadAvatar = new QAction(QIcon(":/gfx/emblem-important.png"), tr("Report inappropriate avatar"), myContextMenu);
	//myContextMenu->addAction(action_ReportBadAvatar);

	//connect( action_VoteForKick, SIGNAL ( triggered() ), this, SLOT ( sendTriggerVoteOnKickSignal() ) );
	//connect( action_IgnorePlayer, SIGNAL ( triggered() ), this, SLOT ( putPlayerOnIgnoreList() ) );
	//connect( action_ReportBadAvatar, SIGNAL ( triggered() ), this, SLOT ( reportBadAvatar() ) );
	//connect( action_EditTip, SIGNAL( triggered() ), this, SLOT ( startEditTip() ) );
}


MyAvatarLabel::~MyAvatarLabel()
{
}

void MyAvatarLabel::contextMenuEvent ( QContextMenuEvent *event )
{

	assert(myW->getSession()->getCurrentGame());

}

void MyAvatarLabel::showContextMenu(const QPoint &pos)
{

	myContextMenu->popup(pos);
}

void MyAvatarLabel::setEnabledContextMenu(bool b)
{
	myContextMenuEnabled = b;
}

void MyAvatarLabel::setPixmap ( const QPixmap &pix, const bool trans)
{

	myPixmap = pix.scaled(30,30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	transparent = trans;
	update();

}

void MyAvatarLabel::setPixmapAndCountry ( const QPixmap &pix,QString countryString, int seatPlace, const bool trans)
{

	QPixmap resultAvatar(pix.scaled(30,30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	QPainter painter(&resultAvatar);
	int showCountryFlags = myW->getConfig()->readConfigInt("ShowCountryFlagInAvatar");
	if(showCountryFlags && !countryString.isEmpty()) {
		if(seatPlace<=2||seatPlace>=8) {
			painter.drawPixmap(resultAvatar.width()-(QPixmap(countryString)).width(),resultAvatar.height()-(QPixmap(countryString)).height(),QPixmap(countryString));
		} else {
			painter.drawPixmap(resultAvatar.width()-(QPixmap(countryString)).width(),0,QPixmap(countryString));
		}
	}
	painter.end();
	myPixmap = resultAvatar;
	transparent = trans;
	update();

}

void MyAvatarLabel::paintEvent(QPaintEvent*)
{

	QPainter painter(this);
	if(transparent)
		painter.setOpacity(0.4);
	else
		painter.setOpacity(1.0);

	painter.drawPixmap(0,0,myPixmap);
}


