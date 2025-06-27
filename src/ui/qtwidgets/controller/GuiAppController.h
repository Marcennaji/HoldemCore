// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>

#include <core/session/Session.h>
#include <infra/persistence/SqliteLogStore.h>

#include <QString>
#include <memory>

namespace pkt::core
{
class Session;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class GuiBridgeWidgets;
class StartWindow;
class GuiAppController
{
  public:
    GuiAppController(const QString& appPath, const QString& logPath, const QString& userDataPath);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    QString myAppDataPath;
    QString myLogPath;
    QString myUserDataPath;
    std::unique_ptr<pkt::infra::SqliteLogStore> myDbStatisticsLogger;
    std::unique_ptr<PokerTableWindow> myPokerTableWindow;
    std::shared_ptr<pkt::core::Session> mySession;
    pkt::core::GameEvents myEvents;
    std::unique_ptr<GuiBridgeWidgets> myBridge;
};
} // namespace pkt::ui::qtwidgets
