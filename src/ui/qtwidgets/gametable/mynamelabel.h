// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYNAMELABEL_H
#define MYNAMELABEL_H

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class GameTableWindow;

class MyNameLabel : public QLabel
{
    Q_OBJECT
  public:
    MyNameLabel(QGroupBox*);
    ~MyNameLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }

  public slots:

    void setText(const QString& t);

  private:
    QString myText;
    GameTableWindow* myW;
};

#endif
