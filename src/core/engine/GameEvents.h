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

    // --- GUI refresh ---
    std::function<void()> onHideHoleCards;
    std::function<void()> onRefreshCash;
    std::function<void()> onRefreshSet;
    std::function<void()> onRefreshAction;
    std::function<void()> onRefreshPlayerName;
    std::function<void()> onRefreshAll;

    // --- Optional: Logging players's actions in the UI (UI log view) ---
    std::function<void(std::string playerName, int action, int setValue)> onLogPlayerAction;
    std::function<void(int gameID, int handID)> onLogNewHand;
    std::function<void(std::string playerName, int pot, bool isMain)> onLogPlayerWins;
    std::function<void(int sbSet, int bbSet, std::string sbName, std::string bbName)> onLogNewBlinds;
    std::function<void(int roundID, int card1, int card2, int card3, int card4, int card5)> onLogDealBoardCards;
    std::function<void(std::string name, int c1, int c2, int value, std::string showType)> onLogFlipHoleCards;
    std::function<void(std::string playerName, int gameID)> onLogGameWin;

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
    std::function<void()> onFlipHolecardsAllIn;

    // --- Optional: UI control ---
    std::function<void()> onDisableButtons;
    std::function<void()> onUpdateMyButtons;
    std::function<void()> onMeInAction;

    void clear()
    {
        *this = GameEvents(); // reset all handlers
    }
};
