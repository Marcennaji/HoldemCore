#include "QmlAppController.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    QmlAppController* controller = new QmlAppController();
    engine.rootContext()->setContextProperty("backend", controller);

#ifdef QT_NO_DEBUG
    // Release: load from resource
    engine.load(QUrl("qrc:/main.qml"));
#else
    // Debug: load from source path (enables hot reload)
    const QString path = QDir::currentPath() + "/main.qml";
    engine.load(QUrl::fromLocalFile(path));
#endif

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
