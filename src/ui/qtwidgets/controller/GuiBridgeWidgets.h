#pragma once

#include <QObject>
#include <core/engine/GameEvents.h>

class GameTableWindow;

class GuiBridgeWidgets : public QObject
{
    Q_OBJECT

  public:
    explicit GuiBridgeWidgets(GameTableWindow* table, QObject* parent = nullptr);
    void connectTo(pkt::core::GameEvents& events);

  private:
    GameTableWindow* myGameTableWindow;
};
