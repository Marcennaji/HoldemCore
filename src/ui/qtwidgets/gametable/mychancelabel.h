// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYCHANCELABEL_H
#define MYCHANCELABEL_H

#include <vector>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QLabel>

class GameTableWindow;
class GameTableStyleReader;

class MyChanceLabel : public QLabel
{
    Q_OBJECT
  public:
    MyChanceLabel(QWidget*);

    ~MyChanceLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }
    void setStyle(GameTableStyleReader* theValue) { myStyle = theValue; }

  private:
    GameTableWindow* myW;
    GameTableStyleReader* myStyle;
};

#endif
