#include "common/common.h"

#include "core/cards/CardRangeAnalyzer.h"

using namespace pkt::core::cards;

namespace pkt::test
{
class IsCardsInRangeTests : public ::testing::Test
{
};

TEST(IsCardsInRangeTests, ExactPair)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("7h", "7d", "77"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("7h", "6d", "77"));
}

TEST(IsCardsInRangeTests, ExactSuitedHand)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Qh", "Jh", "QJs"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Qh", "Jd", "QJs"));
}

TEST(IsCardsInRangeTests, ExactOffsuitedHand)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Kd", "Th", "KTo"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Kd", "Th", "KTs"));
}

TEST(IsCardsInRangeTests, PairAndAbove)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("9h", "9d", "99+"));
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Ah", "Ad", "99+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("8h", "8d", "99+"));
}

TEST(IsCardsInRangeTests, OffsuitedAndAbove)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Ad", "Qh", "AQo+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Ad", "Jh", "AQo+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Ad", "Qh", "AQs+"));
}

TEST(IsCardsInRangeTests, SuitedAndAbove)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Ah", "Jh", "AJs+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Ah", "Jd", "AJs+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Ah", "Jh", "AQs+"));
}

TEST(IsCardsInRangeTests, ExactHand)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Ah", "Kc", "AhKc"));
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Kc", "Ah", "AhKc"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("Ah", "Kd", "AhKc"));
}

TEST(IsCardsInRangeTests, InvalidRange)
{
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("7h", "6h", "7"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("7h", "6h", "777"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("7h", "6h", ""));
}

TEST(IsCardsInRangeTests, MultipleRanges)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("7h", "7d", "77,99+,AQo+"));
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("Ah", "Jh", "77,99+,AJs+"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("8h", "8d", "77,99+,AQo+"));
}

TEST(IsCardsInRangeTests, CardOrder)
{
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("6h", "7h", "76s"));
    EXPECT_TRUE(CardRangeAnalyzer::isCardsInRange("7h", "6h", "76s"));
    EXPECT_FALSE(CardRangeAnalyzer::isCardsInRange("6h", "7h", "76o"));
}

TEST(IsCardsInRangeTests, InvalidCardValues)
{
    // Test that invalid card values throw std::invalid_argument
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("Xh", "7d", "77"), std::invalid_argument);  // Invalid card value
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("7h", "Yd", "77"), std::invalid_argument);  // Invalid card value
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("Zh", "Zd", "99+"), std::invalid_argument); // Invalid card value

    // Test with more invalid card combinations
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("1h", "7d", "77"), std::invalid_argument); // Invalid rank
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("7x", "7d", "77"), std::invalid_argument); // Invalid suit
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("7h", "7z", "77"), std::invalid_argument); // Invalid suit
}

TEST(IsCardsInRangeTests, EmptyCards)
{
    // Test that empty cards also throw std::invalid_argument
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("", "7d", "77"), std::invalid_argument); // Empty card1
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("7h", "", "77"), std::invalid_argument); // Empty card2
    EXPECT_THROW(CardRangeAnalyzer::isCardsInRange("", "", "77"), std::invalid_argument);   // Both cards empty
}

} // namespace pkt::test
