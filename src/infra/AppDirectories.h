
#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <filesystem>
#include <iostream>

namespace pkt::infra
{

namespace fs = std::filesystem;

struct AppDirectories
{
    std::string appDataDir;
    std::string logDir;
    std::string userDataDir;

    static AppDirectories initialize()
    {
        QString basePath = QCoreApplication::instance()->applicationDirPath();
        QString appDir = QDir::cleanPath(basePath) + "/";
        QString log = QDir::cleanPath(appDir + "log-files/") + "/";
        QString data = QDir::cleanPath(appDir + "data/") + "/";

        // Create if not exists
        fs::create_directories(appDir.toStdString());
        fs::create_directories(log.toStdString());
        fs::create_directories(data.toStdString());

        return {appDir.toStdString(), log.toStdString(), data.toStdString()};
    }
};
} // namespace pkt::infra
