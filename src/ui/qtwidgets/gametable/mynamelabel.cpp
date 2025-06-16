// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "mynamelabel.h"
#include <core/engine/Game.h>
#include <core/player/Player.h>
#include <core/session/Session.h>
#include <ui/qtwidgets/styles/gametablestylereader.h>
#include "GameTableWindow.h"

MyNameLabel::MyNameLabel(QGroupBox* parent) : QLabel(parent), myW(0)
{
}

MyNameLabel::~MyNameLabel()
{
}

void MyNameLabel::setText(const QString& t)
{
    QLabel::setText(t);
}
