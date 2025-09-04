#include "HandEvaluator.h"
#include <stdexcept>
#include "core/services/GlobalServices.h"

namespace pkt::core
{

unsigned int HandEvaluator::evaluateHand(const char* hand)
{
    return GlobalServices::instance().handEvaluationEngine().rankHand(hand);
}

} // namespace pkt::core