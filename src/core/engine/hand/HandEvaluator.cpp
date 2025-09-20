#include "HandEvaluator.h"
#include <stdexcept>
#include "core/services/ServiceContainer.h"

namespace pkt::core
{

unsigned int HandEvaluator::evaluateHand(const char* hand)
{
    auto services = std::make_shared<pkt::core::AppServiceContainer>();
    return services->handEvaluationEngine().rankHand(hand);
}

unsigned int HandEvaluator::evaluateHand(const char* hand, std::shared_ptr<pkt::core::ServiceContainer> services)
{
    return services->handEvaluationEngine().rankHand(hand);
}

} // namespace pkt::core