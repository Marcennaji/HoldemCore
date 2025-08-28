#include "test_common.h"

#include "core/player/range/RangeParser.h"

using namespace pkt::core::player;

namespace pkt::test
{
class RangeParserTest : public ::testing::Test
{
};

TEST_F(RangeParserTest, EmptyRanges)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeParserTest, InvalidRangeTooShort)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("A", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeParserTest, InvalidRangeTooLong)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("AAAAA", false);
    EXPECT_TRUE(result.empty());
}

TEST_F(RangeParserTest, ExactPairRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("AA", false);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "AsAd");
    EXPECT_EQ(result[1], "AsAh");
    EXPECT_EQ(result[2], "AsAc");
    EXPECT_EQ(result[3], "AdAh");
    EXPECT_EQ(result[4], "AdAc");
    EXPECT_EQ(result[5], "AcAh");
}

TEST_F(RangeParserTest, ExactSuitedRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("QJs", false);
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "QsJs");
    EXPECT_EQ(result[1], "QdJd");
    EXPECT_EQ(result[2], "QhJh");
    EXPECT_EQ(result[3], "QcJc");
}

TEST_F(RangeParserTest, ExactOffsuitedRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("KTo", false);
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

TEST_F(RangeParserTest, PairAndAboveRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("99+", false);
    EXPECT_EQ(result.size(), 36);
    EXPECT_EQ(result[0], "9s9d");
    EXPECT_EQ(result.back(), "AhAc");
}

TEST_F(RangeParserTest, SuitedAndAboveRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("AQs+", false);
    EXPECT_EQ(result.size(), 8);
    EXPECT_EQ(result[0], "AsQs");
    EXPECT_EQ(result.back(), "AcKc");
}

TEST_F(RangeParserTest, OffsuitedAndAboveRange)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("AQo+", false);
    EXPECT_EQ(result.size(), 24);
    EXPECT_EQ(result[0], "AsQd");
    EXPECT_EQ(result.back(), "AcKh");
}

TEST_F(RangeParserTest, RealHandPair)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("JhJd", false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "JhJd");

    result = RangeParser::getRangeAtomicValues("JhJd", true);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "JJ");
}

TEST_F(RangeParserTest, RealHandNonPair)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("Ad2c", false);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "Ad2c");
}

TEST_F(RangeParserTest, ReturnRangeTrue)
{
    std::vector<std::string> result = RangeParser::getRangeAtomicValues("99+", true);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(result[0], "99");
    EXPECT_EQ(result.back(), "AA");
}

} // namespace pkt::test
