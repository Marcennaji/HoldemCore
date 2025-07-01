#pragma once

#include <functional>
#include <string>
#include <vector>

namespace pkt::core
{

struct GameEvents
{
    std::function<void(int gameSpeed)> onGameInitializeGui;

    std::function<void(int)> onPotUpdated;
    std::function<void(unsigned playerId, int newChips)> onPlayerChipsUpdated;

    std::function<void()> onDealHoleCards;
    std::function<void(unsigned playerId)> onShowHoleCards;
    std::function<void(int bettingRoundId)> onDealBettingRoundCards;
    std::function<void()> onFlipHoleCardsAllIn;

    std::function<void()> onActivePlayerActionDone;

    std::function<void(int)> onRefreshTableDescriptiveLabels;
    std::function<void()> onNextBettingRoundInitializeGui;
    std::function<void()> onHideHoleCards;
    std::function<void()> onRefreshCash;
    std::function<void()> onRefreshSet;
    std::function<void(int playerId, int playerAction)> onRefreshAction;
    std::function<void()> onRefreshPlayerName;
    std::function<void(int playerId, int status)> onRefreshPlayersActiveInactiveStyles;

    std::function<void()> onDisableButtons;
    std::function<void()> onDoHumanAction;

    std::function<void(int bettingRoundId)> onBettingRoundAnimation;

    std::function<void()> onStartPreflop;
    std::function<void()> onStartFlop;
    std::function<void()> onStartTurn;
    std::function<void()> onStartRiver;
    std::function<void()> onStartPostRiver;
    std::function<void()> onPostRiverRunAnimation;
    std::function<void()> onPauseHand;

    void clear()
    {
        *this = GameEvents(); // reset all handlers
    }
};
} // namespace pkt::core
