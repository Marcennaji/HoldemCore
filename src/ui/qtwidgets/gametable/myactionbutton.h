/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the MIT License                                     *


 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * MIT License for more details.                                             *
 *                                                                           *
 * You should have received a copy of the MIT License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#ifndef MYACTIONBUTTON_H
#define MYACTIONBUTTON_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QPushButton>

class GameTableStyleReader;

class MyActionButton : public QPushButton
{
  public:
    MyActionButton(QWidget*);

    ~MyActionButton();

    void paintEvent(QPaintEvent* event);
    void setFKeyText(const QString& theValue);
    void setStyle(GameTableStyleReader* theValue);
    void setEatMyEvents(bool e);
    bool event(QEvent* e);

  private:
    QString fKeyText;
    GameTableStyleReader* myStyle;
    bool eatMyEvents;
};

#endif
