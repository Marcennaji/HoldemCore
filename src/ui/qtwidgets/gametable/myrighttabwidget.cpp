// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "myrighttabwidget.h"
#include <iostream>

using namespace std;

MyRightTabWidget::MyRightTabWidget(QGroupBox* parent) : QTabWidget(parent), myTabBar(0)
{
    myTabBar = this->tabBar();
}

MyRightTabWidget::~MyRightTabWidget()
{
}

void MyRightTabWidget::paintEvent(QPaintEvent* event)
{

    QTabWidget::paintEvent(event);
}
