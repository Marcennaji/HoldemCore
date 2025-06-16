// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef MYCARDSPIXMAPLABEL_H
#define MYCARDSPIXMAPLABEL_H

#include <iostream>

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class GameTableWindow;

class MyCardsPixmapLabel : public QLabel
{
    Q_OBJECT
  public:
    MyCardsPixmapLabel(QGroupBox*);

    ~MyCardsPixmapLabel();

    void setGameTableWindow(GameTableWindow* theValue) { myW = theValue; }

    void setIsFlipside(bool theValue) { isFlipside = theValue; }
    bool getIsFlipside() const { return isFlipside; }

    void setFadeOutAction(bool theValue) { fadeOutAction = theValue; }
    bool getFadeOutAction() const { return fadeOutAction; }

    void startFadeOut(int);
    void stopFadeOut();
    void startFlipCards(int, const QPixmap&, const QPixmap&);
    void stopFlipCardsAnimation();

    void setFlipsidePix(QPixmap p) { flipside = p; }

    void paintEvent(QPaintEvent* event);

  signals:
    void signalFastFlipCards(bool);

  public slots:

    void setPixmap(const QPixmap&, const bool);
    void setHiddenFrontPixmap(const QPixmap&);

    void nextFadeOutFrame();
    void nextFlipCardsFrame();

    void fastFlipCards(bool front);

    // 	void mouseMoveEvent ( QMouseEvent *);

    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

    void setFront(const QPixmap& theValue);

  private:
    GameTableWindow* myW;

    qreal frameOpacity;
    qreal opacityRaiseInterval;

    qreal flipCardsScaleIntervall;
    qreal frameFlipCardsAction1Size;
    qreal frameFlipCardsAction2Size;

    QTimer* fadeOutTimer;
    QTimer* flipCardsTimer;

    bool isFlipside;
    bool fadeOutAction;
    bool flipCardsAction1;
    bool flipCardsAction2;
    bool stopFlipCards;

    bool mousePress;
    bool fastFlipCardsFront;

    QPixmap front;
    QPixmap flipside;
    QPixmap myHiddenFront;

    friend class GameTableWindow;
};

#endif
