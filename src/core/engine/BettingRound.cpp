// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "BettingRound.h"

#include "Exception.h"
#include "model/ButtonState.h"
#include "model/EngineError.h"

#include <core/engine/CardUtilities.h>
#include <core/player/BotPlayer.h>
#include <core/services/GlobalServices.h>

namespace pkt::core
{

using namespace std;
using namespace pkt::core::player;

BettingRound::BettingRound(const GameEvents& events, IHand* hi, unsigned dP, int sB, GameState gS)
    : IBettingRound(), myHand(hi), myBettingRoundId(gS), myDealerPlayerId(dP), mySmallBlind(sB), myMinimumRaise(2 * sB),
      myEvents(events)
{
    PlayerListConstIterator itC;

    // determine bigBlindPosition
    for (itC = myHand->getSeatsList()->begin(); itC != myHand->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getButton() == ButtonBigBlind)
        {
            myBigBlindPlayerId = (*itC)->getId();
            break;
        }
    }
    if (itC == myHand->getSeatsList()->end())
    {
        GlobalServices::instance().logger()->error("BB player not found in the seats list");
    }

    // determine smallBlindPosition
    for (itC = myHand->getSeatsList()->begin(); itC != myHand->getSeatsList()->end(); ++itC)
    {
        if ((*itC)->getButton() == ButtonSmallBlind)
        {
            mySmallBlindPlayerId = (*itC)->getId();
            break;
        }
    }
    if (itC == myHand->getSeatsList()->end() && myHand->getSeatsList()->size() > 2)
    {
        GlobalServices::instance().logger()->error("SB player not found in the seats list");
    }
}

BettingRound::~BettingRound() = default;

void BettingRound::giveActionToNextBotPlayer()
{
    auto nextBotPlayer = myHand->getRunningPlayerFromId(myCurrentPlayersTurnId);
    if (nextBotPlayer == myHand->getRunningPlayersList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }

    auto botPtr = std::dynamic_pointer_cast<BotPlayer>(*nextBotPlayer);
    if (botPtr)
    {
        botPtr->action();
    }
}
void BettingRound::run()
{
    GlobalServices::instance().logger()->verbose("myHighestSet at start of round: " + std::to_string(myHighestSet));
    for (auto& player : *myHand->getRunningPlayersList())
    {
        GlobalServices::instance().logger()->verbose("start of round: Player " + player->getName() +
                                                     " action: " + playerActionToString(player->getAction()));
    }
    if (myFirstRunGui)
    {
        myFirstRunGui = false;
        myHand->setPreviousPlayerId(-1);

        if (myBettingRoundId != 0)
        {
            myHand->resolveHandConditions();
        }

        return;
    }

    if (myFirstRun)
    {
        handleFirstRun();
    }

    logBoardCards();

    bool allHighestSet = allBetsAreDone();
    GlobalServices::instance().logger()->verbose("allBetsAreDone: " + std::to_string(allHighestSet) +
                                                 ", myFirstRound: " + std::to_string(myFirstRound));

    if (!myFirstRound && allHighestSet)
    {
        GlobalServices::instance().logger()->info("all bets are done, proceeding to next betting round");
        proceedToNextBettingRound();
    }
    else
    {
        // determine next running player

        PlayerListConstIterator currentPlayersTurnIt = myHand->getRunningPlayerFromId(myCurrentPlayersTurnId);
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            currentPlayersTurnIt = myHand->getRunningPlayersList()->begin();
        }

        ++currentPlayersTurnIt;
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            currentPlayersTurnIt = myHand->getRunningPlayersList()->begin();
        }

        myCurrentPlayersTurnId = (*currentPlayersTurnIt)->getId();

        if (myEvents.onPlayerStatusChanged)
        {
            myEvents.onPlayerStatusChanged(myCurrentPlayersTurnId, true);
        }

        if (myEvents.onPlayerActed)
        {
            myEvents.onPlayerActed(myCurrentPlayersTurnId, PlayerActionNone);
        }
        GlobalServices::instance().logger()->verbose("BettingRound::run() : Determining next running player");

        currentPlayersTurnIt = myHand->getRunningPlayerFromId(myCurrentPlayersTurnId);
        if (currentPlayersTurnIt == myHand->getRunningPlayersList()->end())
        {
            throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
        }

        (*currentPlayersTurnIt)->setTurn(true);
        GlobalServices::instance().logger()->verbose(
            "BettingRound::run() : Next running player is: " + (*currentPlayersTurnIt)->getName() +
            " with ID: " + std::to_string((*currentPlayersTurnIt)->getId()));

        if (myCurrentPlayersTurnId == myFirstRoundLastPlayersTurnId)
        {
            myFirstRound = false;
        }

        if ((*currentPlayersTurnIt)->getName() == HumanPlayer::getName())
        {
            if (myEvents.onAwaitingHumanInput)
            {
                myEvents.onAwaitingHumanInput();
            }
        }
        else
        {
            GlobalServices::instance().logger()->verbose("Giving action to next bot player: " +
                                                         (*currentPlayersTurnIt)->getName());
            giveActionToNextBotPlayer();
        }
    }
}

bool BettingRound::allBetsAreDone() const
{
    PlayerListIterator itC;

    for (itC = myHand->getRunningPlayersList()->begin(); itC != myHand->getRunningPlayersList()->end(); ++itC)
    {
        if (myHighestSet != (*itC)->getSet())
        {
            return false;
        }
    }

    return true;
}

unsigned BettingRound::findNextEligiblePlayerFromSmallBlind()
{
    GlobalServices::instance().logger()->info("Finding the next eligible player starting from the small blind.");

    PlayerListIterator it = myHand->getPlayerSeatFromId(mySmallBlindPlayerId);
    if (it == myHand->getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::ActivePlayerNotFound);
    }

    // Iterate through the seats list to find the next eligible player
    for (size_t i = 0; i < myHand->getSeatsList()->size(); ++i)
    {
        ++it;
        if (it == myHand->getSeatsList()->end())
        {
            it = myHand->getSeatsList()->begin(); // Wrap around to the beginning
        }

        PlayerListIterator runningPlayerIt = myHand->getRunningPlayerFromId((*it)->getId());
        if (runningPlayerIt != myHand->getRunningPlayersList()->end())
        {
            GlobalServices::instance().logger()->info("Next eligible player found: " + (*it)->getName() +
                                                      " with ID: " + std::to_string((*it)->getId()));
            return (*it)->getId();
        }
    }

    throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
}

void BettingRound::proceedToNextBettingRound()
{
    PlayerListIterator itC;
    myHand->setCurrentRoundState(GameState(myBettingRoundId + 1));

    for (itC = myHand->getRunningPlayersList()->begin(); itC != myHand->getRunningPlayersList()->end(); ++itC)
    {
        (*itC)->setAction(PlayerActionNone);
    }

    GlobalServices::instance().logger()->info("myCurrentPlayersTurnId: " + std::to_string(myCurrentPlayersTurnId));

    myHand->getBoard()->collectSets();
    myHand->getBoard()->collectPot();

    if (myEvents.onPotUpdated)
    {
        myEvents.onPotUpdated(myHand->getBoard()->getPot());
    }

    for (int i = 0; i < MAX_NUMBER_OF_PLAYERS; i++)
    {
        if (myEvents.onPlayerActed)
        {
            myEvents.onPlayerActed(i, PlayerActionNone);
        }
    }

    myHand->resolveHandConditions();
}

void BettingRound::logBoardCards()
{
    if (!myLogBoardCardsDone)
    {
        int tempBoardCardsArray[5];
        myHand->getBoard()->getCards(tempBoardCardsArray);

        switch (myBettingRoundId)
        {
        case GameStateFlop:
            GlobalServices::instance().logger()->info(
                "\n\n************************* FLOP " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + "  *************************\n\n");
            break;
        case GameStateTurn:
            GlobalServices::instance().logger()->info(
                "\n\n************************* TURN " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[3]) + "  *************************\n\n");

            break;
        case GameStateRiver:
            GlobalServices::instance().logger()->info(
                "\n\n************************* RIVER " + CardUtilities::getCardString(tempBoardCardsArray[0]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[1]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[2]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[3]) + " " +
                CardUtilities::getCardString(tempBoardCardsArray[4]) + "  *************************\n\n");

            break;
        default:
            GlobalServices::instance().logger()->error("wrong myBettingRoundId");
        }
        myLogBoardCardsDone = true;
    }
}

void BettingRound::handleFirstRun()
{
    GlobalServices::instance().logger()->info("handleFirstRun: Starting betting round first run logic.");
    myFirstRun = false;
    bool playerFound = false;

    if (myHand->getAllInCondition())
    {
        GlobalServices::instance().logger()->info("All-in condition detected. Skipping first run logic.");
        return;
    }

    GlobalServices::instance().logger()->info("Setting first player to act (clockwise from dealer).");

    // Clockwise from dealer to find first active player
    PlayerListIterator dealerIt = myHand->getPlayerSeatFromId(myDealerPlayerId);
    if (dealerIt == myHand->getSeatsList()->end())
        throw Exception(__FILE__, __LINE__, EngineError::ActivePlayerNotFound);

    // Move to the next position after dealer before starting search
    ++dealerIt;
    if (dealerIt == myHand->getSeatsList()->end())
        dealerIt = myHand->getSeatsList()->begin();

    // Now search for first active player
    for (size_t i = 0; i < myHand->getSeatsList()->size(); ++i)
    {
        int id = (*dealerIt)->getId();
        if (myHand->getRunningPlayerFromId(id) != myHand->getRunningPlayersList()->end())
        {
            myFirstRoundLastPlayersTurnId = id;
            GlobalServices::instance().logger()->info("First player to act: ID " + std::to_string(id));
            playerFound = true;
            break;
        }

        ++dealerIt;
        if (dealerIt == myHand->getSeatsList()->end())
            dealerIt = myHand->getSeatsList()->begin();
    }
    if (!playerFound)
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }

    // Adjust myFirstRoundLastPlayersTurnId to point to the previous player in the running players list
    myFirstRoundLastPlayersTurnId =
        getPreviousRunningPlayerId(myFirstRoundLastPlayersTurnId, myHand->getRunningPlayersList());

    myCurrentPlayersTurnId = myFirstRoundLastPlayersTurnId;
}

unsigned BettingRound::getPreviousRunningPlayerId(unsigned currentPlayerId, PlayerList runningPlayersList) const
{
    // Find the iterator for the current player in the running players list
    PlayerListConstIterator currentPlayerIt = runningPlayersList->begin();
    for (; currentPlayerIt != runningPlayersList->end(); ++currentPlayerIt)
    {
        if ((*currentPlayerIt)->getId() == currentPlayerId)
        {
            break;
        }
    }

    // If the player is not found, throw an exception
    if (currentPlayerIt == runningPlayersList->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::RunningPlayerNotFound);
    }

    // Move to the previous player in the list
    if (currentPlayerIt == runningPlayersList->begin())
    {
        // Wrap around to the last player if at the beginning
        currentPlayerIt = runningPlayersList->end();
    }
    --currentPlayerIt;

    // Return the ID of the previous player
    return (*currentPlayerIt)->getId();
}

GameState BettingRound::getBettingRoundId() const
{
    return myBettingRoundId;
}

void BettingRound::setMinimumRaise(int theValue)
{
    myMinimumRaise = theValue;
}
int BettingRound::getMinimumRaise() const
{
    return myMinimumRaise;
}

void BettingRound::setFullBetRule(bool theValue)
{
    myFullBetRule = theValue;
}
bool BettingRound::getFullBetRule() const
{
    return myFullBetRule;
}

void BettingRound::skipFirstRunGui()
{
    myFirstRunGui = false;
}

IHand* BettingRound::getHand() const
{
    return myHand;
}

void BettingRound::setCurrentPlayersTurnId(unsigned theValue)
{
    myCurrentPlayersTurnId = theValue;
}
unsigned BettingRound::getCurrentPlayersTurnId() const
{
    return myCurrentPlayersTurnId;
}

void BettingRound::setFirstRoundLastPlayersTurnId(unsigned theValue)
{
    myFirstRoundLastPlayersTurnId = theValue;
}
unsigned BettingRound::getFirstRoundLastPlayersTurnId() const
{
    return myFirstRoundLastPlayersTurnId;
}

void BettingRound::setHighestSet(int theValue)
{
    myHighestSet = theValue;
}
int BettingRound::getHighestSet() const
{
    return myHighestSet;
}

void BettingRound::setFirstRun(bool theValue)
{
    myFirstRun = theValue;
}
bool BettingRound::getFirstRun() const
{
    return myFirstRun;
}

void BettingRound::setFirstRound(bool theValue)
{
    myFirstRound = theValue;
}
bool BettingRound::getFirstRound() const
{
    return myFirstRound;
}

unsigned BettingRound::getSmallBlindPlayerId() const
{
    return mySmallBlindPlayerId;
}

unsigned BettingRound::getBigBlindPlayerId() const
{
    return myBigBlindPlayerId;
}

void BettingRound::setSmallBlind(int theValue)
{
    mySmallBlind = theValue;
}
int BettingRound::getSmallBlind() const
{
    return mySmallBlind;
}
void BettingRound::findLastActivePlayerBeforeSmallBlind()
{
    GlobalServices::instance().logger()->verbose("Finding the last active player before the small blind.");

    PlayerListIterator it = getHand()->getPlayerSeatFromId(getSmallBlindPlayerId());
    if (it == getHand()->getSeatsList()->end())
    {
        throw Exception(__FILE__, __LINE__, EngineError::ActivePlayerNotFound);
    }

    if (it == getHand()->getSeatsList()->begin())
    {
        it = getHand()->getSeatsList()->end();
    }
    --it;

    setFirstRoundLastPlayersTurnId((*it)->getId());
    GlobalServices::instance().logger()->verbose("Last active player before small blind found. Player ID: " +
                                                 std::to_string(getFirstRoundLastPlayersTurnId()));
}

bool BettingRound::checkAllHighestSet()
{
    GlobalServices::instance().logger()->verbose("Checking if all running players have the highest set.");

    bool allHighestSet = true;
    for (PlayerListConstIterator itC = getHand()->getRunningPlayersList()->begin();
         itC != getHand()->getRunningPlayersList()->end(); ++itC)
    {
        if (getHighestSet() != (*itC)->getSet())
        {
            allHighestSet = false;
            break;
        }
    }

    GlobalServices::instance().logger()->verbose("All highest set check result: " + std::to_string(allHighestSet));
    return allHighestSet;
}

} // namespace pkt::core
