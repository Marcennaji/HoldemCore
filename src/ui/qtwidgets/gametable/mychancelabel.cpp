// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "mychancelabel.h"
#include <ui/qtwidgets/styles/gametablestylereader.h>
#include "GameTableWindow.h"

using namespace std;

MyChanceLabel::MyChanceLabel(QWidget* parent) : QLabel(parent), myW(0), myStyle(0)
{
}

MyChanceLabel::~MyChanceLabel()
{
}
