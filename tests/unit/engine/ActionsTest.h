#pragma once

#include "common/EngineTest.h"
#include "core/engine/model/PlayerAction.h"

namespace pkt::test
{

class ActionsPreflopTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;

    // Helper methods for creating specific game scenarios
    void setupBasicHeadsUpScenario();
    void setupThreePlayerScenario();
    void setupPlayerWithLimitedCash(int playerId, int cash);
    void simulatePlayerAction(int playerId, pkt::core::ActionType actionType, int amount = 0);

    // Validation helpers
    bool containsAction(const std::vector<pkt::core::ActionType>& actions, pkt::core::ActionType action);
    bool doesNotContainAction(const std::vector<pkt::core::ActionType>& actions, pkt::core::ActionType action);
};

class ActionsPostflopTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;

    // Helper methods for creating specific game scenarios and advancing to post-flop
    void setupBasicHeadsUpScenario();
    void setupThreePlayerScenario();
    void setupPlayerWithLimitedCash(int playerId, int cash);
    void simulatePlayerAction(int playerId, pkt::core::ActionType actionType, int amount = 0);
    void advanceToPostflop(pkt::core::GameState targetState);

    // Validation helpers
    bool containsAction(const std::vector<pkt::core::ActionType>& actions, pkt::core::ActionType action);
    bool doesNotContainAction(const std::vector<pkt::core::ActionType>& actions, pkt::core::ActionType action);
};

} // namespace pkt::test
