#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

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
