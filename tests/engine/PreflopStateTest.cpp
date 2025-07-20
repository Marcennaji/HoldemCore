// tests/PreflopState.cpp

#include "PreflopStateTest.h"
#include "core/engine/model/PlayerAction.h"
#include "core/engine/round_states/PreflopState.h"
#include "core/player/Helpers.h"
#include "core/services/GlobalServices.h"

using namespace pkt::core;
using namespace pkt::core::player;
using namespace std;

namespace pkt::test
{
void PreflopStateTest::logTestMessage(const std::string& message) const
{
    GlobalServices::instance().logger()->verbose("PreflopState : " + message);
}

void PreflopStateTest::SetUp()
{
    EngineTest::SetUp();

    setFlowMode(pkt::core::FlowMode::Fsm);

    myEvents.clear();
}

bool PreflopStateTest::isPlayerStillActive(unsigned id) const
{
    for (const auto& p : *myHand->getRunningPlayersList())
    {
        if (p->getId() == id)
            return true;
    }
    return false;
}
void PreflopStateTest::TearDown()
{
}

TEST_F(PreflopStateTest, GoesToFlopAfterAllPlayersCall)
{
    initializeHandForTesting(3);
    myHand->setCurrentRoundStateFsm(std::make_shared<pkt::core::PreflopState>(myEvents));
}
} // namespace pkt::test
