#pragma once

#include <QObject>
#include <QString>
#include <memory>

class Session;
class SqliteLogStore;

class QmlAppController : public QObject
{
    Q_OBJECT

  public:
    explicit QmlAppController(QObject* parent = nullptr);
    ~QmlAppController();

    Q_INVOKABLE void startGame(int playerCount, int startChips, const QString& profile);
    Q_INVOKABLE void resetGame();

  signals:
    void gameStarted();
    void gameEnded();

  private:
    std::unique_ptr<Session> mySession;
    std::unique_ptr<SqliteLogStore> gameActionslogger;

    void setupBackend();
};
