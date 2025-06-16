// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#include "myactionbutton.h"
#include <ui/qtwidgets/styles/gametablestylereader.h>

using namespace std;

MyActionButton::MyActionButton(QWidget* parent) : QPushButton(parent), myStyle(NULL), eatMyEvents(false)
{
    myStyle = NULL;
}

MyActionButton::~MyActionButton()
{
}

void MyActionButton::setFKeyText(const QString& theValue)
{
    fKeyText = theValue;
}

void MyActionButton::setStyle(GameTableStyleReader* theValue)
{
    assert(theValue != NULL);
    myStyle = theValue;
}

void MyActionButton::setEatMyEvents(bool e)
{
    eatMyEvents = e;
}

void MyActionButton::paintEvent(QPaintEvent* event)
{
    assert(myStyle != NULL);

    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QColor("#" + myStyle->getFKeyIndicatorColor()));
    painter.setOpacity(0.5);
    QFont f = painter.font();
    f.setPixelSize(9);
    painter.setFont(f);
    if (objectName() == ("pushButton_AllIn"))
    {
        painter.drawText(6, 6, 15, 15, Qt::AlignLeft, fKeyText);
    }
    else if (objectName() == ("pushButton_showMyCards"))
    {
        painter.drawText(6, 6, 15, 15, Qt::AlignLeft, QString("F5"));
    }
    else
    {
        painter.drawText(8, 15, 15, 15, Qt::AlignLeft, fKeyText);
    }
}

bool MyActionButton::event(QEvent* event)
{
    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress) && eatMyEvents)
    {
        return true;
    }
    return QWidget::event(event);
}
