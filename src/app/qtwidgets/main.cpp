// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include <ui/qtwidgets/controller/AppController.h>
#include <ui/qtwidgets/theme/ThemeManager.h>
#include <ui/qtwidgets/windows/StartWindow.h>
#include "AppFactory.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets/QApplication>

using namespace std;
using namespace pkt::ui::qtwidgets;

int main(int argc, char** argv)
{
    Q_INIT_RESOURCE(cards);

    QApplication app(argc, argv);

    pkt::ui::qtwidgets::ThemeManager::applyLightTheme(app);

    QCoreApplication::setApplicationName("HoldemCore");
    QCoreApplication::setApplicationVersion("0.9");

    auto controller = pkt::app::qtwidgets::AppFactory::createApplication(
        pkt::app::LoggerType::Console, pkt::app::HandEvaluatorType::Psim, pkt::app::DatabaseType::Sqlite);

    auto* mainWindow = controller->createMainWindow();
    mainWindow->show();

    return app.exec();
}
