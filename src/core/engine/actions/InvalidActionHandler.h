#pragma once

#include <functional>
#include <map>
#include <string>
#include "core/engine/model/PlayerAction.h"

namespace pkt::core
{

struct GameEvents;

/**
 * @brief InvalidActionHandler manages error handling for invalid player actions
 *
 * This class handles tracking invalid actions per player, auto-folding players
 * who repeatedly make invalid actions, and providing error reporting functionality.
 * It encapsulates the logic for dealing with players who don't follow game rules.
 */
class InvalidActionHandler
{
  public:
    using ErrorMessageProvider = std::function<std::string(const PlayerAction&)>;
    using AutoFoldCallback = std::function<void(unsigned playerId)>;

    InvalidActionHandler(const GameEvents& events, ErrorMessageProvider errorProvider,
                         AutoFoldCallback autoFoldCallback);
    ~InvalidActionHandler() = default;

    // Main error handling methods
    void handleInvalidAction(const PlayerAction& action);
    void resetInvalidActionCount(unsigned playerId);
    bool shouldAutoFoldPlayer(unsigned playerId) const;

    // Accessors
    int getInvalidActionCount(unsigned playerId) const;
    int getMaxInvalidActions() const { return MAX_INVALID_ACTIONS; }

  private:
    void executeAutoFold(unsigned playerId);

    const GameEvents& myEvents;
    ErrorMessageProvider myErrorMessageProvider;
    AutoFoldCallback myAutoFoldCallback;

    // Error handling state
    std::map<unsigned, int> myInvalidActionCounts; // Track invalid actions per player
    static const int MAX_INVALID_ACTIONS = 3;      // Max invalid actions before auto-fold
};

} // namespace pkt::core