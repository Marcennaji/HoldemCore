// GuiAppController.h
#pragma once

#include <core/engine/GameEvents.h>

#include <core/session/SessionFsm.h>

#include <QString>
#include <memory>

namespace pkt::core
{
class SessionFsm;
} // namespace pkt::core

namespace pkt::ui::qtwidgets
{
class PokerTableWindow;
class GuiBridgeWidgets;
class StartWindow;
class GuiAppController
{
  public:
    GuiAppController(const QString& appPath);
    ~GuiAppController();

    StartWindow* createMainWindow();

  private:
    QString myAppDataPath;
    std::unique_ptr<PokerTableWindow> myPokerTableWindow;
    std::unique_ptr<pkt::core::SessionFsm> mySession;
    pkt::core::GameEvents myEvents;
    std::unique_ptr<GuiBridgeWidgets> myBridge;
};
} // namespace pkt::ui::qtwidgets
