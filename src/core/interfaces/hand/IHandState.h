#pragma once

#include <string>

namespace pkt::core
{

class HandFsm;

class IHandState
{
  public:
    virtual ~IHandState() = default;

    virtual void enter(HandFsm& hand) = 0;
    virtual void exit(HandFsm& hand) = 0;

    virtual std::string getStateName() const = 0;
};
} // namespace pkt::core
