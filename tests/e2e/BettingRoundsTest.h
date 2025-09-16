#pragma once

#include "common/EngineTest.h"
#include "core/engine/model/GameState.h"

namespace pkt::test
{

class BettingRoundsTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
    bool isPlayerStillActive(unsigned id) const;
    void checkStateTransitions();

    std::vector<core::GameState> stateSequence;
};

} // namespace pkt::test
