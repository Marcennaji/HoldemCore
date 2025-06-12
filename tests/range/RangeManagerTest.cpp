#include "core/player/RangeManager.h"
#include <gtest/gtest.h>

using namespace pkt::core::player;

class RangeManagerTest : public ::testing::Test
{
  protected:
    RangeManager* rangeManager;

    void SetUp() override { rangeManager = new RangeManager(1, nullptr); }

    void TearDown() override { delete rangeManager; }
};

TEST_F(RangeManagerTest, EmptyRanges)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeManagerTest, InvalidRangeTooShort)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("A", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeManagerTest, InvalidRangeTooLong)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("AAAAA", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeManagerTest, ExactPairRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("AA", false);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "AsAd");
    EXPECT_EQ(result[1], "AsAh");
    EXPECT_EQ(result[2], "AsAc");
    EXPECT_EQ(result[3], "AdAh");
    EXPECT_EQ(result[4], "AdAc");
    EXPECT_EQ(result[5], "AcAh");
}

TEST_F(RangeManagerTest, ExactSuitedRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("QJs", false);
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "QsJs");
    EXPECT_EQ(result[1], "QdJd");
    EXPECT_EQ(result[2], "QhJh");
    EXPECT_EQ(result[3], "QcJc");
}

TEST_F(RangeManagerTest, ExactOffsuitedRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("KTo", false);
    EXPECT_EQ(result.size(), 12);
    EXPECT_EQ(result[0], "KsTd");
    EXPECT_EQ(result[1], "KsTc");
    EXPECT_EQ(result[2], "KsTh");
    EXPECT_EQ(result[3], "KdTs");
    EXPECT_EQ(result[4], "KdTc");
    EXPECT_EQ(result[5], "KdTh");
    EXPECT_EQ(result[6], "KhTd");
    EXPECT_EQ(result[7], "KhTc");
    EXPECT_EQ(result[8], "KhTs");
    EXPECT_EQ(result[9], "KcTd");
    EXPECT_EQ(result[10], "KcTs");
    EXPECT_EQ(result[11], "KcTh");
}

TEST_F(RangeManagerTest, PairAndAboveRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("99+", false);
    EXPECT_EQ(result.size(), 36);
    EXPECT_EQ(result[0], "9s9d");
    EXPECT_EQ(result.back(), "AhAc");
}

TEST_F(RangeManagerTest, SuitedAndAboveRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("AQs+", false);
    EXPECT_EQ(result.size(), 8);
    EXPECT_EQ(result[0], "AsQs");
    EXPECT_EQ(result.back(), "AcKc");
}

TEST_F(RangeManagerTest, OffsuitedAndAboveRange)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("AQo+", false);
    EXPECT_EQ(result.size(), 24);
    EXPECT_EQ(result[0], "AsQd");
    EXPECT_EQ(result.back(), "AcKh");
}

TEST_F(RangeManagerTest, RealHandPair)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("JhJd", false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "JhJd");

    result = rangeManager->getRangeAtomicValues("JhJd", true);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "JJ");
}

TEST_F(RangeManagerTest, RealHandNonPair)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("Ad2c", false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "Ad2c");
}

TEST_F(RangeManagerTest, ReturnRangeTrue)
{
    std::vector<std::string> result = rangeManager->getRangeAtomicValues("99+", true);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "99");
    EXPECT_EQ(result.back(), "AA");
}

TEST_F(RangeManagerTest, EmptyOriginRanges)
{
    std::string result = rangeManager->substractRange("", "AJo", "");
    EXPECT_EQ(result, "");
}

TEST_F(RangeManagerTest, EmptyRangesToSubstract)
{
    std::string result = rangeManager->substractRange("AJo", "", "");
    EXPECT_EQ(result, ",AJo");
}

TEST_F(RangeManagerTest, EmptyBoard)
{
    std::string result = rangeManager->substractRange("AJo", "AJo", "");
    EXPECT_EQ(result, "");
}

TEST_F(RangeManagerTest, SubstractSingleRange)
{
    std::string result = rangeManager->substractRange("AJo,KQo", "AJo", "");
    EXPECT_EQ(result, ",KQo");
}

TEST_F(RangeManagerTest, SubstractMultipleRanges)
{
    std::string result = rangeManager->substractRange("AJo,KQo,QJs", "AJo,KQo", "");
    EXPECT_EQ(result, ",QJs");
}

TEST_F(RangeManagerTest, SubstractRealHand)
{
    std::string result = rangeManager->substractRange("AsKs,AhKh", "AsKs", "");
    EXPECT_EQ(result, ",AhKh");
}

TEST_F(RangeManagerTest, SubstractWithBoardCards)
{
    // if the board contains As, then we can't have AsKs in the resulting range, after having substracted AhKh
    // std::string result = rangeManager->substractRange("AsKs,AhKh", "AhKh", "As");
    std::string result = rangeManager->substractRange("AsKs", "", "As");
    EXPECT_EQ(result, "");
}

TEST_F(RangeManagerTest, KeepOriginRange)
{
    std::string result = rangeManager->substractRange("AJo,KQo", "QJs", "");
    EXPECT_EQ(result, ",AJo,KQo");
}

TEST_F(RangeManagerTest, SubstractSuitedRanges)
{
    std::string result = rangeManager->substractRange("AJs,KJs,QJs", "AJs", "");
    EXPECT_EQ(result, ",KJs,QJs");
}

TEST_F(RangeManagerTest, SubstractUnsuitedRanges)
{
    std::string result = rangeManager->substractRange("AJo,KJo,QJo", "KJo", "");
    EXPECT_EQ(result, ",AJo,QJo");
}

TEST_F(RangeManagerTest, SubstractWithAtomicRanges)
{
    std::string result = rangeManager->substractRange("99+,AJo+", "99+", "");
    EXPECT_EQ(result, ",AJo+");
}

TEST_F(RangeManagerTest, SubstractWithBoardImpact)
{
    std::string result = rangeManager->substractRange("AsKs,AhKh", "AhKh", "Ks");
    EXPECT_EQ(result, ",AsKs");
}

TEST_F(RangeManagerTest, SubstractAllRanges)
{
    std::string result = rangeManager->substractRange("AJo,KQo,QJs", "AJo,KQo,QJs", "");
    EXPECT_EQ(result, "");
}

TEST_F(RangeManagerTest, SubstractWithDuplicateRanges)
{
    std::string result = rangeManager->substractRange("AJo,AJo,KQo", "AJo", "");
    EXPECT_EQ(result, ",KQo");
}

TEST_F(RangeManagerTest, SubstractWithComplexBoard)
{
    std::string result = rangeManager->substractRange("AsKs,AhKh,AdKd", "AhKh", "Ks,Ad");
    EXPECT_EQ(result, ",AsKs");
}