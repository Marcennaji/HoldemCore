// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.



#pragma once

#include "common/EngineTest.h"

namespace pkt::test
{

class FlopStateTest : public EngineTest
{
  protected:
    void SetUp() override;
    void TearDown() override;
    void logTestMessage(const std::string& message) const;
};
} // namespace pkt::test