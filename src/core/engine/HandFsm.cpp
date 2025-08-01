#include "HandFsm.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "core/player/Helpers.h"
#include "model/ButtonState.h"

#include "Exception.h"
#include "model/EngineError.h"

#include "core/engine/round_states/PreflopState.h"
#include "core/player/typedefs.h"

#include <random>

namespace pkt::core
{
using namespace std;
using namespace pkt::core::player;

HandFsm::HandFsm(const GameEvents& events, std::shared_ptr<EngineFactory> factory, std::shared_ptr<IBoard> board,
                 PlayerFsmList seats, PlayerFsmList runningPlayers, GameData gameData, StartData startData)
    : myEvents(events), myFactory(factory), myBoard(board), mySeatsList(seats), myRunningPlayersList(runningPlayers),
      myStartQuantityPlayers(startData.numberOfPlayers), myDealerPlayerId(startData.startDealerPlayerId),
      mySmallBlindPlayerId(startData.startDealerPlayerId), myBigBlindPlayerId(startData.startDealerPlayerId),
      mySmallBlind(gameData.firstSmallBlind), myStartCash(gameData.startMoney)

{
}

HandFsm::~HandFsm() = default;

void HandFsm::start()
{
    GlobalServices::instance().logger()->verbose(
        "\n----------------------  New hand initialization (FSM)  -------------------------------\n");

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards();
    dealHoleCards(cardsArrayIndex);

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->resetForNewHand();
    }

    myPreflopLastRaiserId = -1;

    myState = std::make_unique<PreflopState>(myEvents, mySmallBlind, myDealerPlayerId);
    myState->enter(*this);
}

void HandFsm::end()
{
    myState.reset();
}

void HandFsm::handlePlayerAction(PlayerAction action)
{
    if (!myState)
        return;

    if (auto* processor = dynamic_cast<IActionProcessor*>(myState.get()))
    {
        if (!processor->isActionAllowed(*this, action))
            return;

        applyActionEffects(action);

        auto next = processor->computeNextState(*this, action);
        if (next)
        {
            myState->exit(*this);
            myState = std::move(next);
            myState->enter(*this);
        }
    }
}
void HandFsm::applyActionEffects(const PlayerAction& action)
{
    auto player = getPlayerFsmById(myRunningPlayersList, action.playerId);
    auto* bettingState = dynamic_cast<BettingStateBase*>(myState.get());

    switch (action.type)
    {
    case ActionType::Fold:
        updateRunningPlayersList();
        break;

    case ActionType::Call:
    {
        player->getLegacyPlayer()->setSet(action.amount);

        // myPot += toCall;
        break;
    }

    case ActionType::Raise:
    {
        /*int toCall = currentBet() - player.currentBet();
        int raiseAmount = action.amount();
        int totalPutIn = toCall + raiseAmount;

        player.stack -= totalPutIn;
        player.addToCurrentBet(totalPutIn);
        myPot += totalPutIn;
        myCurrentBet = player.currentBet(); // update table's highest bet
        myLastAggressor = player.id();
        */
        break;
    }
    case ActionType::None:
    {
        break;
    }
    case ActionType::Bet:
    {
        break;
    }
    case ActionType::Check:
    {
        break;
    }
    case ActionType::Allin:
    {
        break;
    }
    }

    // myActionHistory.push_back(action);
}

const pkt::core::player::PlayerFsmList HandFsm::getSeatsList() const
{
    return mySeatsList;
}
const pkt::core::player::PlayerFsmList HandFsm::getRunningPlayersList() const
{
    return myRunningPlayersList;
}

void HandFsm::initAndShuffleDeck()
{
    myCardsArray = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
                    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 rng(rd()); // Seed the Mersenne Twister random number generator
    std::shuffle(myCardsArray.begin(), myCardsArray.end(), rng);
}

void HandFsm::dealHoleCards(size_t cardsArrayIndex)
{
    int boardCardIndex, holeCardIndex, playerIndex = 0;
    int tempPlayerArray[2];
    int tempPlayerAndBoardArray[7];

    // Validate that there are enough cards in the deck
    if (myCardsArray.size() < 5 + 2 * mySeatsList->size())
    {
        throw std::runtime_error("Not enough cards in the deck to deal hole cards and board cards.");
    }

    // Initialize the first 5 cards of the board
    for (boardCardIndex = 0; boardCardIndex < 5; boardCardIndex++)
    {
        tempPlayerAndBoardArray[boardCardIndex] = myCardsArray[boardCardIndex];
    }

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it, playerIndex++)
    {
        for (holeCardIndex = 0; holeCardIndex < 2; holeCardIndex++)
        {
            tempPlayerArray[holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
            tempPlayerAndBoardArray[5 + holeCardIndex] = myCardsArray[2 * playerIndex + holeCardIndex + 5];
        }
        string humanReadableHand = CardUtilities::getCardStringValue(tempPlayerAndBoardArray, 7);
        (*it)->getLegacyPlayer()->setCards(tempPlayerArray);
        (*it)->getLegacyPlayer()->setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str()));
        GlobalServices::instance().logger()->verbose(
            "Player " + (*it)->getLegacyPlayer()->getName() + " dealt cards: " +
            CardUtilities::getCardString(tempPlayerArray[0]) + " " + CardUtilities::getCardString(tempPlayerArray[1]) +
            ", hand strength = " + std::to_string((*it)->getLegacyPlayer()->getHandRanking()));
    }
}

size_t HandFsm::dealBoardCards()
{
    int tempBoardArray[5];
    size_t cardsArrayIndex = 0;

    for (size_t i = 0; i < 5; ++i) // The board consists of 5 cards (Flop, Turn, River)
    {
        tempBoardArray[i] = myCardsArray[cardsArrayIndex++];
    }

    myBoard->setCards(tempBoardArray);
    return cardsArrayIndex;
}
void HandFsm::updateRunningPlayersList()
{
    GlobalServices::instance().logger()->verbose("Updating myRunningPlayersList...");

    PlayerFsmListIterator it, it1;

    for (it = myRunningPlayersList->begin(); it != myRunningPlayersList->end();)
    {
        GlobalServices::instance().logger()->verbose(
            "Checking player: " + (*it)->getLegacyPlayer()->getName() +
            ", action: " + playerActionToString((*it)->getLegacyPlayer()->getAction()));

        if ((*it)->getLegacyPlayer()->getAction() == ActionType::Fold ||
            (*it)->getLegacyPlayer()->getAction() == ActionType::Allin)
        {
            GlobalServices::instance().logger()->verbose("Removing player: " + (*it)->getLegacyPlayer()->getName() +
                                                         " from myRunningPlayersList due to action: " +
                                                         playerActionToString((*it)->getLegacyPlayer()->getAction()));

            it = myRunningPlayersList->erase(it);

            if (!myRunningPlayersList->empty())
            {
                GlobalServices::instance().logger()->verbose(
                    "myRunningPlayersList is not empty after removal. Updating current player's turn.");

                it1 = it;
                if (it1 == myRunningPlayersList->begin())
                {
                    GlobalServices::instance().logger()->verbose(
                        "Iterator points to the beginning of the list. Wrapping around to the end.");
                    it1 = myRunningPlayersList->end();
                }
                --it1;
            }
            else
            {
                GlobalServices::instance().logger()->verbose("myRunningPlayersList is now empty after removal.");
            }
        }
        else
        {
            GlobalServices::instance().logger()->verbose(
                "Player: " + (*it)->getLegacyPlayer()->getName() +
                " remains in myRunningPlayersList. Moving to the next player.");
            ++it;
        }
    }

    GlobalServices::instance().logger()->verbose("Finished updating myRunningPlayersList.");
}

} // namespace pkt::core
