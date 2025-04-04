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
#ifndef CARDDECKSTYLEREADER_H
#define CARDDECKSTYLEREADER_H

#include <third_party/tiny/tinyxml.h>
#include <configfile.h>
#include <string>
#include <QtCore>
#include <QtGui>

#define POKERTRAINING_CD_STYLE_FILE_VERSION	1

enum CdStyleState {
	CD_STYLE_OK = 0,
	CD_STYLE_OUTDATED,
	CD_STYLE_FIELDS_EMPTY,
	CD_STYLE_PICTURES_MISSING,
};

class CardDeckStyleReader : public QObject
{
	Q_OBJECT
public:
	CardDeckStyleReader(ConfigFile *c, QWidget *w );
	~CardDeckStyleReader();

	void readStyleFile(QString);

	QString getStyleDescription() const {
		return StyleDescription;
	}
	QString getStyleMaintainerName() const {
		return StyleMaintainerName;
	}
	QString getStyleMaintainerEMail() const	{
		return StyleMaintainerEMail;
	}
	QString getStyleCreateDate() const {
		return StyleCreateDate;
	}

	QString getCurrentFileName() const {
		return currentFileName;
	}
	QString getCurrentDir() const {
		return currentDir;
	}

	QString getPreview() const {
		return Preview;
	}

	bool getFallBack() const {
		return fallBack;
	}
	bool getLoadedSuccessfull() const {
		return loadedSuccessfull;
	}

	void showLeftItemsErrorMessage(QString, QStringList, QString);
	void showCardsLeftErrorMessage(QString, QStringList, QString);

private:

	QString StyleDescription;
	QString StyleMaintainerName;
	QString StyleMaintainerEMail;
	QString StyleCreateDate;
	QString PokerTrainingStyleFileVersion;
	QString Preview;

	QString currentFileName;
	QString currentDir;
	QByteArray fileContent;

	QStringList cardsLeft;
	QStringList leftItems;

	ConfigFile *myConfig;
	QWidget *myW;

	bool fallBack;
	bool loadedSuccessfull;
};

#endif
