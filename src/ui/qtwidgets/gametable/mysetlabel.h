// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYSETLABEL_H
#define MYSETLABEL_H

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>

class GameTableWindow;

class MySetLabel : public QLabel
{
    Q_OBJECT
  public:
    MySetLabel(QGroupBox*);

    ~MySetLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }
    void paintEvent(QPaintEvent* event);

  public slots:

  private:
    GameTableWindow* myW;
};

#endif
