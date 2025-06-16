// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
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
