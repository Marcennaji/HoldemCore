#pragma once

#include "core/interfaces/hand/IHandState.h"

namespace pkt::core
{

class BettingStateBase : public IHandState
{
  protected:
    int myHighestSet = 0;

  public:
    int getHighestSet() const { return myHighestSet; }

    void updateHighestSet(int amount)
    {
        if (amount > myHighestSet)
            myHighestSet = amount;
    }

    void resetHighestSet() { myHighestSet = 0; }

    virtual void enter(HandFsm& hand) = 0;
    virtual void exit(HandFsm& hand) = 0;
    virtual std::string getStateName() const = 0;
};

} // namespace pkt::core
