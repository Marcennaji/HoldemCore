// NullLogger.h

#pragma once
#include <core/interfaces/ILogger.h>

namespace pkt::infra
{

class NullLogger : public core::ILogger
{
  public:
    virtual void error(const std::string& msg) override {}
    virtual void info(const std::string& msg) override {}
    virtual void verbose(const std::string& msg, int level = 1) override {}
};

} // namespace pkt::infra
