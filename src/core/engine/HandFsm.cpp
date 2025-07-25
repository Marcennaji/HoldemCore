#include "HandFsm.h"
#include <core/services/GlobalServices.h>
#include "CardUtilities.h"
#include "GameEvents.h"
#include "Randomizer.h"
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

    GlobalServices::instance().logger()->verbose(
        "\n----------------------  New hand initialization (FSM)  -------------------------------\n");

    for (auto it = mySeatsList->begin(); it != mySeatsList->end(); ++it)
    {
        (*it)->getLegacyPlayer().setHand(nullptr);
        (*it)->getLegacyPlayer().setCardsFlip(0);
    }

    initAndShuffleDeck();
    size_t cardsArrayIndex = dealBoardCards(); // we need to deal the board first,
                                               // so that the players can use it to evaluate their hands
    dealHoleCards(cardsArrayIndex);

    myPreflopLastRaiserId = -1;

    // determine dealer, SB, BB
    // assignButtons();

    // setBlinds();
}

HandFsm::~HandFsm() = default;

void HandFsm::start()
{
    myState = std::make_unique<PreflopState>(myEvents);
}

void HandFsm::end()
{
    myState.reset();
}

void HandFsm::handlePlayerAction(int playerId, PlayerAction action)
{
    if (!myState)
        return;

    if (auto* processor = dynamic_cast<IActionProcessor*>(myState.get()))
    {
        if (!processor->canProcessAction(*this, action))
        {
            // Handle invalid action
            return;
        }

        auto next = processor->processAction(*this, action);
        if (next)
        {
            myState->exit(*this);
            myState = std::move(next);
            myState->enter(*this);
        }
    }
}
pkt::core::player::PlayerFsmList HandFsm::getSeatsList() const
{
    return mySeatsList;
}
pkt::core::player::PlayerFsmList HandFsm::getRunningPlayersList() const
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
        (*it)->getLegacyPlayer().setCards(tempPlayerArray);
        (*it)->getLegacyPlayer().setHandRanking(HandEvaluator::evaluateHand(humanReadableHand.c_str()));
        (*it)->getLegacyPlayer().setCashAtHandStart((*it)->getLegacyPlayer().getCash());
        (*it)->getLegacyPlayer().getCurrentHandActions().reset();
        (*it)->getLegacyPlayer().setPosition();
        (*it)->getLegacyPlayer().getRangeEstimator()->setEstimatedRange("");
        GlobalServices::instance().logger()->verbose(
            "Player " + (*it)->getLegacyPlayer().getName() + " dealt cards: " +
            CardUtilities::getCardString(tempPlayerArray[0]) + " " + CardUtilities::getCardString(tempPlayerArray[1]) +
            ", hand strength = " + std::to_string((*it)->getLegacyPlayer().getHandRanking()));
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

} // namespace pkt::core
