
#pragma once

#include "common/EngineTest.h"

namespace pkt::test
{

class CardsTest : public EngineTest
{

  public:
    bool cardsAreUniqueAndValid(const std::shared_ptr<pkt::core::IHand>& hand,
                                const std::shared_ptr<pkt::core::IBoard>& board,
                                const pkt::core::player::PlayerList& players);
};
} // namespace pkt::test