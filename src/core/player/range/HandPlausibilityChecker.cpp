
#include "HandPlausibilityChecker.h"

#include "FlopPlausibilityChecker.h"
#include "RiverPlausibilityChecker.h"
#include "TurnPlausibilityChecker.h"

namespace pkt::core::player
{

// Flop methods - delegate to FlopPlausibilityChecker
bool HandPlausibilityChecker::isUnplausibleHandGivenFlopCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return FlopPlausibilityChecker::isUnplausibleHandGivenFlopCheck(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopBet(const PostFlopAnalysisFlags& possibleSituation,
                                                            const CurrentHandContext& ctx)
{
    return FlopPlausibilityChecker::isUnplausibleHandGivenFlopBet(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopCall(const PostFlopAnalysisFlags& possibleSituation,
                                                             const CurrentHandContext& ctx)
{
    return FlopPlausibilityChecker::isUnplausibleHandGivenFlopCall(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return FlopPlausibilityChecker::isUnplausibleHandGivenFlopRaise(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenFlopAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return FlopPlausibilityChecker::isUnplausibleHandGivenFlopAllin(possibleSituation, ctx);
}

// Turn methods - delegate to TurnPlausibilityChecker
bool HandPlausibilityChecker::isUnplausibleHandGivenTurnCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return TurnPlausibilityChecker::isUnplausibleHandGivenTurnCheck(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnBet(const PostFlopAnalysisFlags& possibleSituation,
                                                            const CurrentHandContext& ctx)
{
    return TurnPlausibilityChecker::isUnplausibleHandGivenTurnBet(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnCall(const PostFlopAnalysisFlags& possibleSituation,
                                                             const CurrentHandContext& ctx)
{
    return TurnPlausibilityChecker::isUnplausibleHandGivenTurnCall(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return TurnPlausibilityChecker::isUnplausibleHandGivenTurnRaise(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenTurnAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return TurnPlausibilityChecker::isUnplausibleHandGivenTurnAllin(possibleSituation, ctx);
}

// River methods - delegate to RiverPlausibilityChecker
bool HandPlausibilityChecker::isUnplausibleHandGivenRiverCheck(const PostFlopAnalysisFlags& possibleSituation,
                                                               const CurrentHandContext& ctx)
{
    return RiverPlausibilityChecker::isUnplausibleHandGivenRiverCheck(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverBet(const PostFlopAnalysisFlags& possibleSituation,
                                                             const CurrentHandContext& ctx)
{
    return RiverPlausibilityChecker::isUnplausibleHandGivenRiverBet(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverCall(const PostFlopAnalysisFlags& possibleSituation,
                                                              const CurrentHandContext& ctx)
{
    return RiverPlausibilityChecker::isUnplausibleHandGivenRiverCall(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverRaise(const PostFlopAnalysisFlags& possibleSituation,
                                                               const CurrentHandContext& ctx)
{
    return RiverPlausibilityChecker::isUnplausibleHandGivenRiverRaise(possibleSituation, ctx);
}

bool HandPlausibilityChecker::isUnplausibleHandGivenRiverAllin(const PostFlopAnalysisFlags& possibleSituation,
                                                               const CurrentHandContext& ctx)
{
    return RiverPlausibilityChecker::isUnplausibleHandGivenRiverAllin(possibleSituation, ctx);
}

} // namespace pkt::core::player
