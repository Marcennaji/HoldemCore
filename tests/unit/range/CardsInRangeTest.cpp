#include "common/common.h"

#include "core/player/Helpers.h"

using namespace pkt::core::player;

namespace pkt::test
{
class IsCardsInRangeTests : public ::testing::Test
{
};

TEST(IsCardsInRangeTests, ExactPair)
{
    EXPECT_TRUE(isCardsInRange("7h", "7d", "77"));
    EXPECT_FALSE(isCardsInRange("7h", "6d", "77"));
}

TEST(IsCardsInRangeTests, ExactSuitedHand)
{
    EXPECT_TRUE(isCardsInRange("Qh", "Jh", "QJs"));
    EXPECT_FALSE(isCardsInRange("Qh", "Jd", "QJs"));
}

TEST(IsCardsInRangeTests, ExactOffsuitedHand)
{
    EXPECT_TRUE(isCardsInRange("Kd", "Th", "KTo"));
    EXPECT_FALSE(isCardsInRange("Kd", "Th", "KTs"));
}

TEST(IsCardsInRangeTests, PairAndAbove)
{
    EXPECT_TRUE(isCardsInRange("9h", "9d", "99+"));
    EXPECT_TRUE(isCardsInRange("Ah", "Ad", "99+"));
    EXPECT_FALSE(isCardsInRange("8h", "8d", "99+"));
}

TEST(IsCardsInRangeTests, OffsuitedAndAbove)
{
    EXPECT_TRUE(isCardsInRange("Ad", "Qh", "AQo+"));
    EXPECT_FALSE(isCardsInRange("Ad", "Jh", "AQo+"));
    EXPECT_FALSE(isCardsInRange("Ad", "Qh", "AQs+"));
}

TEST(IsCardsInRangeTests, SuitedAndAbove)
{
    EXPECT_TRUE(isCardsInRange("Ah", "Jh", "AJs+"));
    EXPECT_FALSE(isCardsInRange("Ah", "Jd", "AJs+"));
    EXPECT_FALSE(isCardsInRange("Ah", "Jh", "AQs+"));
}

TEST(IsCardsInRangeTests, ExactHand)
{
    EXPECT_TRUE(isCardsInRange("Ah", "Kc", "AhKc"));
    EXPECT_TRUE(isCardsInRange("Kc", "Ah", "AhKc"));
    EXPECT_FALSE(isCardsInRange("Ah", "Kd", "AhKc"));
}

TEST(IsCardsInRangeTests, InvalidRange)
{
    EXPECT_FALSE(isCardsInRange("7h", "6h", "7"));
    EXPECT_FALSE(isCardsInRange("7h", "6h", "777"));
    EXPECT_FALSE(isCardsInRange("7h", "6h", ""));
}

TEST(IsCardsInRangeTests, MultipleRanges)
{
    EXPECT_TRUE(isCardsInRange("7h", "7d", "77,99+,AQo+"));
    EXPECT_TRUE(isCardsInRange("Ah", "Jh", "77,99+,AJs+"));
    EXPECT_FALSE(isCardsInRange("8h", "8d", "77,99+,AQo+"));
}

TEST(IsCardsInRangeTests, CardOrder)
{
    EXPECT_TRUE(isCardsInRange("6h", "7h", "76s"));
    EXPECT_TRUE(isCardsInRange("7h", "6h", "76s"));
    EXPECT_FALSE(isCardsInRange("6h", "7h", "76o"));
}

TEST(IsCardsInRangeTests, InvalidCardValues)
{
    // Test that invalid card values throw std::invalid_argument
    EXPECT_THROW(isCardsInRange("Xh", "7d", "77"), std::invalid_argument);  // Invalid card value
    EXPECT_THROW(isCardsInRange("7h", "Yd", "77"), std::invalid_argument);  // Invalid card value
    EXPECT_THROW(isCardsInRange("Zh", "Zd", "99+"), std::invalid_argument); // Invalid card value

    // Test with more invalid card combinations
    EXPECT_THROW(isCardsInRange("1h", "7d", "77"), std::invalid_argument); // Invalid rank
    EXPECT_THROW(isCardsInRange("7x", "7d", "77"), std::invalid_argument); // Invalid suit
    EXPECT_THROW(isCardsInRange("7h", "7z", "77"), std::invalid_argument); // Invalid suit
}

TEST(IsCardsInRangeTests, EmptyCards)
{
    // Test that empty cards also throw std::invalid_argument
    EXPECT_THROW(isCardsInRange("", "7d", "77"), std::invalid_argument); // Empty card1
    EXPECT_THROW(isCardsInRange("7h", "", "77"), std::invalid_argument); // Empty card2
    EXPECT_THROW(isCardsInRange("", "", "77"), std::invalid_argument);   // Both cards empty
}

} // namespace pkt::test
