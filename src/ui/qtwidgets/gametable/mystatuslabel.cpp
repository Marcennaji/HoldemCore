// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "GameTableWindow.h"
#include "mysetlabel.h"

using namespace std;

MyStatusLabel::MyStatusLabel(QGroupBox* parent) : QLabel(parent), myW(NULL), mousePress(false)
{

    mousePress = false;
}

MyStatusLabel::~MyStatusLabel()
{
}

void MyStatusLabel::mousePressEvent(QMouseEvent* event)
{

    if (!mousePress && objectName().contains("textLabel_Status0"))
    {
        mousePress = true;
    }

    QLabel::mousePressEvent(event);
}

void MyStatusLabel::mouseReleaseEvent(QMouseEvent* event)
{

    if (mousePress && objectName().contains("textLabel_Status0"))
    {
        mousePress = false;
    }

    QLabel::mouseReleaseEvent(event);
}
