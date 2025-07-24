#pragma once

namespace pkt::core
{

class IHand;

class IRoundCompletionChecker
{
  public:
    virtual ~IRoundCompletionChecker() = default;

    virtual bool isRoundComplete(const IHand& hand) const = 0;
};

} // namespace pkt::core
