#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>

class GameTableWindow;

class GuiBridgeWidgets : public QObject
{
    Q_OBJECT

  public:
    explicit GuiBridgeWidgets(GameTableWindow* table, QObject* parent = nullptr);
    void connectTo(GameEvents& events);

  private:
    GameTableWindow* myGameTableWindow;
};
