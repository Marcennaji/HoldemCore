#pragma once
#include <core/interfaces/ILogger.h>
#include <iostream>
#include <string>

class ConsoleLogger : public ILogger {
    int logLevel;
public:
    explicit ConsoleLogger(int level = 1);

    void error(const std::string& msg) override;
    void info(const std::string& msg) override;
    void verbose(const std::string& msg, int level = 2) override;
};
