#pragma once

namespace pkt::core
{

class HandFsm;

class IRoundCompletionChecker
{
  public:
    virtual ~IRoundCompletionChecker() = default;

    virtual bool isRoundComplete(const HandFsm& hand) const = 0;
};

} // namespace pkt::core
