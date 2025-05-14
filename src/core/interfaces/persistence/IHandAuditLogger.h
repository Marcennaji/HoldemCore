#pragma once
#include <string>

class Hand;   
class Range;

class IHandAuditLogger {
public:
    virtual ~IHandAuditLogger() = default;

    virtual void updateUnplausibleHand(const std::string& playerName,
                                     const Hand& actualHand,
                                     const Range& estimatedRange) = 0;
};
