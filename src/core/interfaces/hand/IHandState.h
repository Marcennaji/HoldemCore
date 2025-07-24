#pragma once

#include <string>

namespace pkt::core
{

class IHand;

class IHandState
{
  public:
    virtual ~IHandState() = default;

    virtual void enter(IHand& hand) = 0;
    virtual void exit(IHand& hand) = 0;

    virtual std::string getStateName() const = 0;
};
} // namespace pkt::core
