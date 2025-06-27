#include "HandEvaluator.h"
#include <stdexcept>
#include "core/services/GlobalServices.h"

namespace pkt::core
{

unsigned int HandEvaluator::evaluateHand(const char* hand)
{
    auto& services = GlobalServices::instance();
    auto engine = services.handEvaluationEngine();

    if (!engine)
    {
        throw std::runtime_error(
            "No hand evaluation engine configured. Call GlobalServices::instance().setHandEvaluationEngine() first.");
    }

    services.logger()->info("Evaluating hand with " + std::string(engine->getEngineName()) +
                            " engine: " + std::string(hand));

    unsigned int result = engine->rankHand(hand);

    services.logger()->info("Hand evaluation result: " + std::to_string(result));

    return result;
}

} // namespace pkt::core