// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "core/interfaces/IBoard.h"

#include "core/player/typedefs.h"

namespace pkt::core::player
{
class Player;

} // namespace pkt::core::player
namespace pkt::core
{

class IHand;

class Board : public IBoard
{
  public:
    Board(unsigned dealerPosition);
    ~Board();

    void setSeatsList(pkt::core::player::PlayerList seats);
    void setRunningPlayersList(pkt::core::player::PlayerList runningPlayers);

    void setCards(int* theValue);
    void getCards(int* theValue);

    void setAllInCondition(bool theValue);
    void setLastActionPlayerId(unsigned theValue);

    int getPot() const;
    void setPot(int theValue);
    int getSets() const;
    void setSets(int theValue);

    void collectSets();
    void collectPot();

    void distributePot();
    void determinePlayerNeedToShowCards();

    std::list<unsigned> getWinners() const;
    void setWinners(const std::list<unsigned>& w);

    std::list<unsigned> getPlayerNeedToShowCards() const;
    void setPlayerNeedToShowCards(const std::list<unsigned>& p);

  private:
    std::shared_ptr<player::Player> getPlayerById(unsigned id) const;

    // -- -distributePot helper methods-- -
    std::shared_ptr<player::Player> distributePotResolveRemainderReceiver(const std::vector<unsigned>& winnerIds) const;
    pkt::core::player::PlayerListIterator getSeatsIt(unsigned playerId) const;
    std::vector<unsigned> distributePotInitializePlayerContributions();
    bool distributePotHasRemaining(const std::vector<unsigned>& contributions) const;
    unsigned distributePotNextLevelAmount(const std::vector<unsigned>& contributions) const;
    std::vector<size_t> distributePotEligibleContributors(const std::vector<unsigned>& contributions,
                                                          unsigned levelAmount) const;
    std::vector<size_t> distributePotDetermineWinners(const std::vector<size_t>& eligibleIndexes,
                                                      unsigned levelAmount) const;
    void distributePotAwardBase(const std::vector<size_t>& winnerIndexes, int baseAmount);
    void distributePotDistributeRemainder(const std::vector<size_t>& winnerIndexes, int remainder);
    void distributePotReduceContributions(std::vector<unsigned>& contributions, unsigned amount);
    void distributePotFinalize();

    pkt::core::player::PlayerList mySeatsList;
    pkt::core::player::PlayerList myRunningPlayersList;

    std::list<unsigned> myWinners;
    std::list<unsigned> myPlayerNeedToShowCards;

    int myCards[5];
    int myPot{0};
    int mySets{0};
    unsigned myDealerPlayerId;
    bool myAllInCondition{false};
    unsigned myLastActionPlayerId{0};
};

} // namespace pkt::core
