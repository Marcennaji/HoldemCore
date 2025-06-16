// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "mysetlabel.h"

#include "GameTableWindow.h"

using namespace std;

MySetLabel::MySetLabel(QGroupBox* parent) : QLabel(parent), myW(0)
{
}

MySetLabel::~MySetLabel()
{
}

void MySetLabel::paintEvent(QPaintEvent* event)
{

    QLabel::paintEvent(event);
}
