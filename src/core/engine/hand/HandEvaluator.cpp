#include "HandEvaluator.h"
#include <stdexcept>
#include "core/services/ServiceContainer.h"

namespace pkt::core
{

unsigned int HandEvaluator::evaluateHand(const char* hand)
{
    // Prefer DI overload; fallback uses a shared default services instance to avoid repeated allocations.
    static std::shared_ptr<pkt::core::ServiceContainer> s_defaultServices =
        std::make_shared<pkt::core::AppServiceContainer>();
    return s_defaultServices->handEvaluationEngine().rankHand(hand);
}

unsigned int HandEvaluator::evaluateHand(const char* hand, std::shared_ptr<pkt::core::ServiceContainer> services)
{
    return services->handEvaluationEngine().rankHand(hand);
}

} // namespace pkt::core