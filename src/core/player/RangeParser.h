#pragma once
#include <string>
#include <vector>

namespace pkt::core::player
{

class RangeParser
{
  public:
    // convert a range into a list of real cards, or a list of simplified ranges
    // if returnRange is true, the range is processed and split, only to remove the + signs (if any)
    // otherwise, if returnRange is false, it returns the atomic values of the range (the "real cards")
    static std::vector<std::string> getRangeAtomicValues(const std::string& ranges, const bool returnRange = false);

  private:
    static void handleExactPair(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleThreeCharRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleFourCharRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static void handleSuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                  std::vector<std::string>& result);
    static void handleOffsuitedRange(const std::string& s1, const std::string& s2, const bool returnRange,
                                     std::vector<std::string>& result);
    static void handlePairAndAboveRange(char c, const bool returnRange, std::vector<std::string>& result);
    static void handleOffsuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                             std::vector<std::string>& result);
    static void handleSuitedAndAboveRange(const std::string& s1, char c, const bool returnRange,
                                          std::vector<std::string>& result);
    static void handleExactPairRange(const char* range, const bool returnRange, std::vector<std::string>& result);
    static char incrementCardValue(char c);
};

} // namespace pkt::core::player
