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
#include "mycashlabel.h"
#include <ui/qtwidgets/styles/gametablestylereader.h>
#include "GameTableWindow.h"

MyCashLabel::MyCashLabel(QGroupBox* parent) : QLabel(parent), myW(0)
{
}

MyCashLabel::~MyCashLabel()
{
}

void MyCashLabel::setText(const QString& t, bool trans)
{
    QColor transColor;
    transColor.setNamedColor("#" + myW->getGameTableStyle()->getPlayerCashTextColor());
    QString red = QString::number(transColor.red());
    QString green = QString::number(transColor.green());
    QString blue = QString::number(transColor.blue());

    if (trans)
    {
        this->setStyleSheet("QLabel { " + myW->getGameTableStyle()->getFont2String() +
                            " font-size: " + myW->getGameTableStyle()->getPlayerCashLabelFontSize() +
                            "px; font-weight: bold; color: rgba(" + red + ", " + green + ", " + blue + ", 80); }");
    }
    else
    {
        this->setStyleSheet("QLabel { " + myW->getGameTableStyle()->getFont2String() + " font-size: " +
                            myW->getGameTableStyle()->getPlayerCashLabelFontSize() + "px; font-weight: bold; color: #" +
                            myW->getGameTableStyle()->getPlayerCashTextColor() + "; }");
    }

    QLabel::setText(t);
}
