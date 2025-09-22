// PlayerActionValidator.cpp
#include "PlayerActionValidator.h"
#include <algorithm>

namespace pkt::ui::qtwidgets
{

PlayerActionValidator::ValidationResult PlayerActionValidator::validateBettingAction(
    int amount, 
    pkt::core::ActionType actionType,
    int playerId,
    int playerChips,
    int minimumBet,
    const std::vector<pkt::core::ActionType>& validActions) const
{
    ValidationResult result;
    result.correctedAction.playerId = playerId;
    result.correctedAction.type = actionType;
    result.correctedAction.amount = amount;

    // Check if betting action is allowed
    if (!isActionValid(actionType, validActions)) {
        result.isValid = false;
        result.errorMessage = QString("Action %1 is not allowed right now.")
                                .arg(actionTypeToString(actionType));
        return result;
    }

    // Validate amount constraints
    if (amount < 0) {
        result.isValid = false;
        result.errorMessage = "Betting amount cannot be negative.";
        return result;
    }

    if (amount > playerChips) {
        result.isValid = false;
        result.errorMessage = QString("You cannot bet more than your chips ($%1).").arg(playerChips);
        
        // Suggest all-in if player wants to bet more than they have
        result.correctedAction.type = pkt::core::ActionType::Allin;
        result.correctedAction.amount = playerChips;
        return result;
    }

    if (amount > 0 && amount < minimumBet) {
        result.isValid = false;
        result.errorMessage = QString("Minimum bet is $%1. Your bet of $%2 is too small.")
                                .arg(minimumBet).arg(amount);
        
        // Suggest corrected amount
        result.correctedAction.amount = minimumBet;
        return result;
    }

    // All validations passed
    result.isValid = true;
    result.errorMessage = "";
    return result;
}

PlayerActionValidator::ValidationResult PlayerActionValidator::validateGeneralAction(
    pkt::core::ActionType actionType,
    int playerId,
    const std::vector<pkt::core::ActionType>& validActions) const
{
    ValidationResult result;
    result.correctedAction.playerId = playerId;
    result.correctedAction.type = actionType;
    result.correctedAction.amount = 0;

    if (!isActionValid(actionType, validActions)) {
        result.isValid = false;
        result.errorMessage = QString("Action %1 is not allowed right now.")
                                .arg(actionTypeToString(actionType));
        return result;
    }

    result.isValid = true;
    result.errorMessage = "";
    return result;
}

QString PlayerActionValidator::getActionDescription(const pkt::core::PlayerAction& action) const
{
    QString description = QString("Player %1 ").arg(action.playerId);
    
    switch (action.type) {
        case pkt::core::ActionType::Fold:
            description += "folds";
            break;
        case pkt::core::ActionType::Check:
            description += "checks";
            break;
        case pkt::core::ActionType::Call:
            description += "calls";
            break;
        case pkt::core::ActionType::Bet:
            description += QString("bets $%1").arg(action.amount);
            break;
        case pkt::core::ActionType::Raise:
            description += QString("raises to $%1").arg(action.amount);
            break;
        case pkt::core::ActionType::Allin:
            description += QString("goes all-in with $%1").arg(action.amount);
            break;
        default:
            description += "makes an unknown action";
            break;
    }
    
    return description;
}

bool PlayerActionValidator::isActionValid(pkt::core::ActionType actionType, const std::vector<pkt::core::ActionType>& validActions) const
{
    return std::find(validActions.begin(), validActions.end(), actionType) != validActions.end();
}

int PlayerActionValidator::correctBettingAmount(int amount, int minimumBet, int maximumBet) const
{
    return std::clamp(amount, minimumBet, maximumBet);
}

} // namespace pkt::ui::qtwidgets