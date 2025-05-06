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
#include "aboutPokerTrainingimpl.h"
#include <configfile.h>
#include "game_defs.h"
#include <QtCore>


aboutPokerTrainingImpl::aboutPokerTrainingImpl(QWidget *parent, ConfigFile *c)
	: QDialog(parent), myConfig(c)
{
	setupUi(this);

	myAppDataPath = QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str());

	QPalette myPalette = textBrowser_licence->palette();

	QColor myColor = myPalette.color(QPalette::Window);
	myPalette.setColor(QPalette::Base, myColor);

	textBrowser_licence->setPalette(myPalette);
	textBrowser_2->setPalette(myPalette);
	textBrowser_3->setPalette(myPalette);
        textBrowser_4->setPalette(myPalette);

	QFile gplFile(QDir::toNativeSeparators(myAppDataPath+"misc/agpl.html"));
	QString gplString;
	if(gplFile.exists()) {
		if (gplFile.open( QIODevice::ReadOnly)) {
			QTextStream stream( &gplFile );
			gplString = stream.readAll();
			textBrowser_licence->setHtml(gplString);
		}
	}


	label_logo->setPixmap(QPixmap(":/gfx/logoChip3D.png"));

	label_PokerTrainingVersion->setStyleSheet("QLabel { font-size: 16px; font-weight: bold;}");

	label_PokerTrainingVersion->setText(QString(tr("PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));
	this->setWindowTitle(QString(tr("About PokerTraining %1").arg(POKERTRAINING_BETA_RELEASE_STRING)));

     //add text to lables and textbrowsers
	QString thxToInfos;
	thxToInfos.append(tr("- PokerTH developers, for the GUI part and poker rules implementation")+"<br>");
	thxToInfos.append(tr("- Wikimedia Commons: for different popular avatar picture resources")+"<br>");
    thxToInfos.append(tr("- danuxi: for startwindow background gfx and danuxi1 table background")+"<br>");
	textBrowser_3->setHtml(thxToInfos);

    QString infoText;
    infoText.append(tr("- Poker software for Texas Hold'em No Limit Poker")+"\n");
    infoText.append(tr("- Singleplayer games with up to 9 computer-opponents")+"\n");
    infoText.append("\n");
    infoText.append("(c)2013, Marc Ennaji");
    label_infotext->setText(infoText);

    QString projectText;
    projectText.append("<b>"+tr("Project page:")+"</b><br>");
    projectText.append("&nbsp;&nbsp;&nbsp;&nbsp;<a href='http://sourceforge.net/projects/pokertraining/'>http://sourceforge.net/projects/pokertraining/</a><br>");
     projectText.append("<b>"+tr("Author:")+"</b><br>");
    projectText.append("&nbsp;&nbsp;&nbsp;&nbsp;Marc Ennaji");
    textBrowser_2->setHtml(projectText);
}
