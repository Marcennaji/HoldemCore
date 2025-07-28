#pragma once

#include <optional>
#include "core/interfaces/hand/IHandState.h"

namespace pkt::core
{
class IPlayerAccess;

class BettingStateBase : public IHandState
{

  public:
    bool isRoundComplete(const HandFsm& hand) const;
    int getHighestSet() const;
    void updateHighestSet(int amount);
    void resetHighestSet() { myHighestSet = 0; }

    virtual void enter(HandFsm& hand) = 0;
    virtual void exit(HandFsm& hand) = 0;
    virtual std::string getStateName() const = 0;

    void recordRaise(unsigned int id) { myLastRaiserId = id; }

    std::optional<unsigned int> getLastRaiserId() const { return myLastRaiserId; }

    void resetRaiser() { myLastRaiserId = std::nullopt; }

  protected:
    bool haveAllPlayersCalledOrFolded(const HandFsm& hand) const;
    bool isOnlyOnePlayerRemaining(const HandFsm& hand) const;

    int myHighestSet = 0;
    std::optional<unsigned int> myLastRaiserId = std::nullopt;
};

} // namespace pkt::core
