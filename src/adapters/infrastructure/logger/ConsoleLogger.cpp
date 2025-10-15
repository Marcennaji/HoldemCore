#include "ConsoleLogger.h"

#include <iostream>

namespace pkt::infra
{

using namespace pkt::core;

ConsoleLogger::ConsoleLogger()
{
    m_logLevel = LogLevel::Info;
}

void ConsoleLogger::error(const std::string& msg)
{
    std::cerr << "[ERROR] " << msg << std::endl;
}

void ConsoleLogger::info(const std::string& msg)
{
    if (m_logLevel >= LogLevel::Info)
    {
        std::cout << "[INFO] " << msg << std::endl;
    }
}

void ConsoleLogger::decisionMaking(const std::string& msg)
{
    if (m_logLevel >= LogLevel::DecisionMaking)
    {
        std::cout << "[DECISION] " << msg << std::endl;
    }
}

void ConsoleLogger::verbose(const std::string& msg)
{
    if (m_logLevel >= LogLevel::Verbose)
    {
        std::cout << "[VERBOSE] " << msg << std::endl;
    }
}

void ConsoleLogger::debug(const std::string& msg)
{
    if (m_logLevel >= LogLevel::Debug)
    {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}
} // namespace pkt::infra
