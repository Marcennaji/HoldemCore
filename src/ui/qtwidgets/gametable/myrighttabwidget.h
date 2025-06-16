// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYRIGHTTABWIDGET_H
#define MYRIGHTTABWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QTabWidget>

class MyRightTabWidget : public QTabWidget
{
    Q_OBJECT
  public:
    MyRightTabWidget(QGroupBox*);

    ~MyRightTabWidget();

    void paintEvent(QPaintEvent* event);

    QTabBar* getTabBar() const { return myTabBar; }

  public slots:

  private:
    QTabBar* myTabBar;
};

#endif
