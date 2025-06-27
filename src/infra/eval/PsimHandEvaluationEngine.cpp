#include "PsimHandEvaluationEngine.h"
#include <third_party/psim/psim.hpp>

namespace pkt::infra
{

unsigned int PsimHandEvaluationEngine::rankHand(const char* hand)
{
    return ::rankHand(hand);
}

} // namespace pkt::infra