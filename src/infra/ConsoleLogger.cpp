#include "ConsoleLogger.h"

#include <iostream>

namespace pkt::infra
{

ConsoleLogger::ConsoleLogger(int level) : myLogLevel(level)
{
}

void ConsoleLogger::error(const std::string& msg)
{
    std::cerr << "[ERROR] " << msg << std::endl;
}

void ConsoleLogger::info(const std::string& msg)
{
    if (myLogLevel >= 1)
        std::cout << "[INFO] " << msg << std::endl;
}

void ConsoleLogger::verbose(const std::string& msg, int level)
{
    if (myLogLevel >= level)
        std::cout << "[VERBOSE] " << msg << std::endl;
}
} // namespace pkt::infra
