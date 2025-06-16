// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYCASHLABEL_H
#define MYCASHLABEL_H

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class GameTableWindow;

class MyCashLabel : public QLabel
{
    Q_OBJECT
  public:
    MyCashLabel(QGroupBox*);
    ~MyCashLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }

  public slots:

    void setText(const QString&, bool = false);

  private:
    QString myText;
    GameTableWindow* myW;
};

#endif
