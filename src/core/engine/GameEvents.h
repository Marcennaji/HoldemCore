#pragma once

#include <functional>
#include <string>
#include <vector>

struct GameEvents
{
    // --- Game state display ---
    std::function<void(int)> onPotUpdated;
    std::function<void(unsigned playerId, int newChips)> onPlayerChipsUpdated;

    // --- Card dealing ---
    std::function<void()> onDealHoleCards;
    std::function<void(unsigned playerId)> onShowHoleCards;
    std::function<void(int bettingRoundId)> onDealBettingRoundCards;

    std::function<void()> onActivePlayerActionDone;

    // --- GUI refresh ---
    std::function<void(int)> onRefreshTableDescriptiveLabels;
    std::function<void()> onNextBettingRoundInitializeGui;
    std::function<void()> onHideHoleCards;
    std::function<void()> onRefreshCash;
    std::function<void()> onRefreshSet;
    std::function<void(int playerID, int playerAction)> onRefreshAction;
    std::function<void()> onRefreshPlayerName;
    std::function<void(int playerID, int status)> onRefreshPlayersActiveInactiveStyles;

    std::function<void()> onDisableButtons;
    std::function<void()> onDoHumanAction;

    // --- Optional: animations ---
    std::function<void()> onPreflopAnimation1;
    std::function<void()> onPreflopAnimation2;
    std::function<void()> onFlopAnimation1;
    std::function<void()> onFlopAnimation2;
    std::function<void()> onTurnAnimation1;
    std::function<void()> onTurnAnimation2;
    std::function<void()> onRiverAnimation1;
    std::function<void()> onRiverAnimation2;
    std::function<void()> onPostRiverAnimation1;
    std::function<void()> onPostRiverRunAnimation1;
    std::function<void()> onFlipHoleCardsAllIn;

    void clear()
    {
        *this = GameEvents(); // reset all handlers
    }
};
