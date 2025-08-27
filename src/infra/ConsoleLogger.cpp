#include "ConsoleLogger.h"

#include <iostream>

namespace pkt::infra
{

using namespace pkt::core;

ConsoleLogger::ConsoleLogger()
{
    myLogLevel = LogLevel::Info;
}

void ConsoleLogger::error(const std::string& msg)
{
    std::cerr << "[ERROR] " << msg << std::endl;
}

void ConsoleLogger::info(const std::string& msg)
{
    if (myLogLevel >= LogLevel::Info)
    {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void ConsoleLogger::verbose(const std::string& msg)
{
    if (myLogLevel >= LogLevel::Verbose)
    {
        std::cout << "[VERBOSE] " << msg << std::endl;
    }
}

void ConsoleLogger::debug(const std::string& msg)
{
    if (myLogLevel >= LogLevel::Debug)
    {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}
} // namespace pkt::infra
