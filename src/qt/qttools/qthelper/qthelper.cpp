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

#ifdef _WIN32
	path += "/data/";
#else
#ifdef __APPLE__
	if (QRegExp("Contents/MacOS/?$").indexIn(path) != -1) {
		// pointing into an macosx application bundle
		path += "/../Resources/data/";
	} else {
		path += "/data/";
	}
#else //Unix
    QRegularExpression rx;
    rx.setPattern("PokerTraining/?$");
    QRegularExpressionMatch match = rx.match(path);
    if (match.hasMatch())
    {
		// there is an own application directory
		path += "/data/";
    } else
    {
        rx.setPattern("usr/games/bin/?$");
        match = rx.match(path);
        if (match.hasMatch())
        {
            // we are in /usr/games/bin (like gentoo linux does)
            path += "/../../share/games/PokerTraining/data/";
        }else
        {
            rx.setPattern("usr/games/?$");
            match = rx.match(path);
            if (match.hasMatch())
            {
                // we are in /usr/games (like Debian linux does)
                path += "/../share/games/PokerTraining/";
            }
            else
            {
                    rx.setPattern("bin/?$");
                    match = rx.match(path);
                    if (match.hasMatch())
                    {
                        // we are in a bin directory. e.g. /usr/bin
                        path += "/../share/PokerTraining/data/";
                    }
                    else
                    {
                            path += "/data/";
                    }
            }
        }
    }
#endif
#endif
	return (QDir::cleanPath(path) + "/").toStdString();
}
// [01:09] <Zhenech> doitux|mob, mach den pfad als define, und nur wenns nich gesetzt is wildes raten
// [01:10] <Zhenech> dann compilieren die distries mit -DDATAPTH="/usr/share/games/PokerTraining" o.ä.
// [01:10] <Zhenech> und du suchst eine liste ab:
// [01:10] <Zhenech> ist es in [/usr/share/PokerTraining, /usr/share/games/PokerTraining/, /usr/local/..., $PWD/data]



