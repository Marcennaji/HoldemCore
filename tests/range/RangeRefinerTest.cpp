#include "core/player/RangeRefiner.h"
#include <gtest/gtest.h>

using namespace pkt::core::player;

class RangeRefinerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Setup code if needed
    }

    void TearDown() override
    {
        // Cleanup code if needed
    }
};

TEST_F(RangeRefinerTest, EmptyOriginRanges)
{
    std::string result = RangeRefiner::substractRange("", "AJo", "AsKs8c10d");
    EXPECT_EQ(result, "");
}

TEST_F(RangeRefinerTest, EmptyRangesToSubstract)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo", "", "AsKs2d");
    EXPECT_EQ(result, "AJo,KQo");
}

TEST_F(RangeRefinerTest, EmptyBoard)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo", "AJo", "");
    EXPECT_EQ(result, "KQo");
}

TEST_F(RangeRefinerTest, SubstractSingleRange)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo,QJs", "AJo", "AsKs2d");
    EXPECT_EQ(result, "KQo,QJs");
}

TEST_F(RangeRefinerTest, SubstractMultipleRanges)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo,QJs", "AJo,KQo", "AsKs2d");
    EXPECT_EQ(result, "QJs");
}

TEST_F(RangeRefinerTest, SubstractRealHand)
{
    std::string result = RangeRefiner::substractRange("AsKs,AhKh", "AsKs", "AdKd2d");
    EXPECT_EQ(result, "AhKh");
}

TEST_F(RangeRefinerTest, SubstractWithBoardCards)
{
    std::string result = RangeRefiner::substractRange("AsKs,AhKh", "AhKh", "As2d5d");
    EXPECT_EQ(result, "");
}

TEST_F(RangeRefinerTest, KeepOriginRange)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo", "QJs", "AsKs4d");
    EXPECT_EQ(result, "AJo,KQo");
}

TEST_F(RangeRefinerTest, SubstractSuitedRanges)
{
    std::string result = RangeRefiner::substractRange("AJs,KJs,QJs", "AJs", "AsKs8d2h5d");
    EXPECT_EQ(result, "KdJd,KhJh,KcJc,QJs");
}

TEST_F(RangeRefinerTest, SubstractUnsuitedRanges)
{
    std::string result = RangeRefiner::substractRange("AJo,KJo,QJo", "KJo", "AsKs2d");
    EXPECT_EQ(result, "AJo,QJo");
}

TEST_F(RangeRefinerTest, SubstractWithAtomicRanges)
{
    std::string result = RangeRefiner::substractRange("99+,AJo+", "99+", "AsKs2d");
    EXPECT_EQ(result, "AJo,AQo,AKo");
}

TEST_F(RangeRefinerTest, SubstractWithBoardImpact)
{
    std::string result = RangeRefiner::substractRange("AsKs,AhKh", "AhKh", "Ks2d2c");
    EXPECT_EQ(result, "");
}

TEST_F(RangeRefinerTest, SubstractAllRanges)
{
    std::string result = RangeRefiner::substractRange("AJo,KQo,QJs", "AJo,KQo,QJs", "AsKs2d");
    EXPECT_EQ(result, "");
}

TEST_F(RangeRefinerTest, SubstractWithDuplicateRanges)
{
    std::string result = RangeRefiner::substractRange("AJo,AJo,KQo", "AJo", "AsKs2d");
    EXPECT_EQ(result, "KQo");
}
