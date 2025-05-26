#pragma once
#include <string>

namespace pkt::core
{

class ILogger
{
  public:
    virtual ~ILogger() = default;
    virtual void error(const std::string& msg) = 0;
    virtual void info(const std::string& msg) = 0;
    virtual void verbose(const std::string& msg, int level = 1) = 0;
};

} // namespace pkt::core
