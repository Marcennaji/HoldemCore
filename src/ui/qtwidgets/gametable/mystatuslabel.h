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
#ifndef MYSTATUSLABEL_H
#define MYSTATUSLABEL_H

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class GameTableWindow;

class MyStatusLabel : public QLabel
{
    Q_OBJECT
  public:
    MyStatusLabel(QGroupBox*);

    ~MyStatusLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

  private:
    GameTableWindow* myW;
    bool mousePress;
};

#endif
