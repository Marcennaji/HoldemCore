#include "HandEvaluator.h"
#include <stdexcept>

namespace pkt::core
{

unsigned int HandEvaluator::evaluateHand(const char* hand, std::shared_ptr<pkt::core::HandEvaluationEngine> engine)
{
    if (engine) {
        return engine->rankHand(hand);
    }else{
        throw std::runtime_error("HandEvaluator::evaluateHand: No valid HandEvaluationEngine provided");
    }
}

} // namespace pkt::core