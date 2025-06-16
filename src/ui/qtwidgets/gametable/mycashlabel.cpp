// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
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
