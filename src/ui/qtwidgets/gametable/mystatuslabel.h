// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
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
