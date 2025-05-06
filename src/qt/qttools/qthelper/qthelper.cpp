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
#include "qthelper.h"
#include <QtCore>
#include <iostream>
#include <QRegularExpression>

QtHelper::QtHelper()
{
}


QtHelper::~QtHelper()
{
}

std::string QtHelper::stringToUtf8(const std::string &myString)
{

	QString tmpString = QString::fromLocal8Bit(myString.c_str());
	std::string myUtf8String = tmpString.toUtf8().constData();

	return myUtf8String;
}

std::string QtHelper::stringFromUtf8(const std::string &myString)
{
	QString tmpString = QString::fromUtf8(myString.c_str());

	return tmpString.toLocal8Bit().constData();
}

std::string QtHelper::getDefaultLanguage()
{
	return QLocale::system().name().toStdString();
}

std::string QtHelper::getDataPathStdString(const char * /*argv0*/)
{
	QString path(QCoreApplication::instance()->applicationDirPath());
	path += "/data/";

	return (QDir::cleanPath(path) + "/").toStdString();
}



