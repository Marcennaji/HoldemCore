#include "common/common.h"
#include "core/engine/hand/HandEvaluator.h"
#include "core/services/GlobalServices.h"
#include "infra/ConsoleLogger.h"
#include "infra/eval/PsimHandEvaluationEngine.h"

using namespace pkt::core;
namespace pkt::test
{
class CardsValueTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto& services = pkt::core::GlobalServices::instance();
        services.setLogger(std::make_unique<pkt::infra::ConsoleLogger>());
        services.setHandEvaluationEngine(std::make_unique<pkt::infra::PsimHandEvaluationEngine>());
    }

    void TearDown() override {}

    // Helper function to evaluate a hand
    unsigned int evaluateHand(const std::vector<std::string>& boardCards, const std::vector<std::string>& holeCards)
    {
        // Combine board and hole cards into a single string
        std::string cards = "";
        for (const auto& card : boardCards)
        {
            cards += card + " ";
        }
        for (const auto& card : holeCards)
        {
            cards += card + " ";
        }

        return HandEvaluator::evaluateHand(cards.c_str());
    }
};

// Test case: Compare two hands with the same board cards
TEST_F(CardsValueTest, CompareTwoHands)
{

    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    std::vector<std::string> holeCards1 = {"Th", "8h"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    std::vector<std::string> holeCards2 = {"7c", "2d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    EXPECT_GT(result2, result1);
}

// Test case: Compare multiple hands with the same board cards
TEST_F(CardsValueTest, CompareMultipleHands)
{

    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    std::vector<std::string> holeCards1 = {"Th", "8h"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    std::vector<std::string> holeCards2 = {"7c", "2d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    std::vector<std::string> holeCards3 = {"9c", "Ac"};
    unsigned int result3 = evaluateHand(boardCards, holeCards3);

    std::vector<std::string> holeCards4 = {"3d", "8s"};
    unsigned int result4 = evaluateHand(boardCards, holeCards4);

    std::vector<std::string> holeCards5 = {"Jh", "6c"};
    unsigned int result5 = evaluateHand(boardCards, holeCards5);

    std::vector<std::string> holeCards6 = {"7h", "5c"};
    unsigned int result6 = evaluateHand(boardCards, holeCards6);

    // Assertions based on the expected results
    EXPECT_GT(result2, result1); // Player 2 > Player 1
    EXPECT_GT(result3, result2); // Player 3 > Player 2
    EXPECT_GT(result4, result1); // Player 4 > Player 1
    EXPECT_GT(result5, result1); // Player 5 > Player 1
    EXPECT_EQ(result2, result6); // Player 2 == Player 6
}

// Test case: Ensure hand evaluation is consistent
TEST_F(CardsValueTest, ConsistentEvaluation)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    std::vector<std::string> holeCards = {"9c", "Ac"};

    // Evaluate the same hand multiple times
    unsigned int result1 = evaluateHand(boardCards, holeCards);
    unsigned int result2 = evaluateHand(boardCards, holeCards);

    // The results should be consistent
    EXPECT_EQ(result1, result2);
}

// Test case: Edge case with low-value hole cards
TEST_F(CardsValueTest, LowValueHoleCards)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "5h"};

    std::vector<std::string> holeCards = {"2d", "3d"};
    unsigned int result = evaluateHand(boardCards, holeCards);

    // Ensure the result is valid (non-zero)
    EXPECT_GT(result, 0);
}
// Test case: Compare hands where the best combination uses 0, 1, or 2 hole cards
TEST_F(CardsValueTest, BestCombinationUsesDifferentHoleCards)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    // Best combination uses 0 hole cards (board cards only)
    std::vector<std::string> holeCards1 = {"2d", "3d"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Best combination uses 1 hole card
    std::vector<std::string> holeCards2 = {"9c", "2d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Best combination uses 2 hole cards
    std::vector<std::string> holeCards3 = {"9c", "Ac"};
    unsigned int result3 = evaluateHand(boardCards, holeCards3);

    // Assertions
    EXPECT_GT(result2, result1); // 1 hole card > 0 hole cards
    EXPECT_GT(result3, result2); // 2 hole cards > 1 hole card
}

// Test case: Compare hands with equal rankings
TEST_F(CardsValueTest, EqualRankingsWithZeroHoleCard)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "10h"};

    std::vector<std::string> holeCards1 = {"2d", "4d"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    std::vector<std::string> holeCards2 = {"2c", "3c"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Assertions
    EXPECT_EQ(result1, result2);
}
TEST_F(CardsValueTest, EqualRankingsWithOneHoleCard)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "10h"};

    std::vector<std::string> holeCards1 = {"2d", "Kd"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    std::vector<std::string> holeCards2 = {"2c", "Kc"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Assertions
    EXPECT_EQ(result1, result2);
}
// Test case: Compare hands where the best combination uses 0 hole cards
TEST_F(CardsValueTest, BestCombinationUsesZeroHoleCards)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "6h"};

    // Both hands rely only on board cards
    std::vector<std::string> holeCards1 = {"2d", "3d"};
    std::vector<std::string> holeCards2 = {"4d", "5d"};

    unsigned int result1 = evaluateHand(boardCards, holeCards1);
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Both hands should have the same ranking
    EXPECT_EQ(result1, result2);
}

// Test case: Compare hands where the best combination uses 1 hole card
TEST_F(CardsValueTest, BestCombinationUsesOneHoleCard)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "6h"};

    // Hand 1 uses 1 hole card
    std::vector<std::string> holeCards1 = {"9c", "2d"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Hand 2 uses 1 hole card
    std::vector<std::string> holeCards2 = {"9d", "3d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Both hands should have the same ranking
    EXPECT_EQ(result1, result2);
}

// Test case: Compare hands where the best combination uses 2 hole cards
TEST_F(CardsValueTest, BestCombinationUsesTwoHoleCards)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    // Hand 1 uses 2 hole cards
    std::vector<std::string> holeCards1 = {"9c", "Ac"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Hand 2 uses 2 hole cards
    std::vector<std::string> holeCards2 = {"9d", "Ad"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Both hands should have the same ranking
    EXPECT_EQ(result1, result2);
}
// Test case: Compare hands with flushes
TEST_F(CardsValueTest, HandsWithFlushes)
{
    std::vector<std::string> boardCards = {"2h", "4h", "6h", "8h", "Th"};

    // Hand 1 forms a flush using the board cards
    std::vector<std::string> holeCards1 = {"2d", "3d"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Hand 2 forms a flush using one hole card
    std::vector<std::string> holeCards2 = {"Ah", "3d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Hand 3 forms a flush using two hole cards
    std::vector<std::string> holeCards3 = {"Ah", "Kh"};
    unsigned int result3 = evaluateHand(boardCards, holeCards3);

    // Assertions
    EXPECT_GT(result2, result1); // Flush with one hole card > flush with board cards
    EXPECT_GT(result3, result2); // Flush with two hole cards > flush with one hole card
}

// Test case: Compare hands with straights
TEST_F(CardsValueTest, HandsWithStraights)
{
    /**/
    std::vector<std::string> boardCards = {"5h", "6d", "7s", "8c", "9h"};

    // Hand 1 forms a straight using the board cards
    std::vector<std::string> holeCards1 = {"2d", "Td"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Hand 2 forms a straight using one hole card
    std::vector<std::string> holeCards2 = {"4d", "3d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Hand 3 forms a straight using two hole cards
    std::vector<std::string> holeCards3 = {"Td", "Jd"};
    unsigned int result3 = evaluateHand(boardCards, holeCards3);

    // Assertions
    EXPECT_GT(result1, result2); // Straight with one hole card > straight with board cards
    EXPECT_GT(result3, result2); // Straight with two hole cards > straight with one hole card
}

// Test case: Compare hands with full houses
TEST_F(CardsValueTest, HandsWithFullHouses)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "7h", "3h"};

    // Hand 1 forms a full house using the board cards
    std::vector<std::string> holeCards1 = {"2d", "3d"};
    unsigned int result1 = evaluateHand(boardCards, holeCards1);

    // Hand 2 forms a full house using one hole card
    std::vector<std::string> holeCards2 = {"9c", "3d"};
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Hand 3 forms a full house using two hole cards
    std::vector<std::string> holeCards3 = {"9c", "7c"};
    unsigned int result3 = evaluateHand(boardCards, holeCards3);

    // Assertions
    EXPECT_GT(result2, result1); // Full house with one hole card > full house with board cards
    EXPECT_EQ(result3, result2); // Full house with two hole cards > full house with one hole card
}

// Test case: Hands with ties
TEST_F(CardsValueTest, HandsWithTies)
{
    std::vector<std::string> boardCards = {"9s", "9h", "7s", "Ks", "3h"};

    // Both hands have the same ranking but different suits
    std::vector<std::string> holeCards1 = {"9c", "Ac"};
    std::vector<std::string> holeCards2 = {"9d", "Ad"};

    unsigned int result1 = evaluateHand(boardCards, holeCards1);
    unsigned int result2 = evaluateHand(boardCards, holeCards2);

    // Both hands should have the same ranking
    EXPECT_EQ(result1, result2);
}
} // namespace pkt::test
