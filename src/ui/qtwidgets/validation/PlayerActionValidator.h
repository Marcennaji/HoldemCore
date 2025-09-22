// PlayerActionValidator.h
#pragma once

#include <core/engine/model/PlayerAction.h>
#include <QString>
#include <vector>

namespace pkt::ui::qtwidgets
{

/**
 * @brief Validates user input and provides feedback for poker actions
 * 
 * This class handles client-side validation before sending actions to the game engine.
 * It provides immediate user feedback and prevents obviously invalid actions.
 */
class PlayerActionValidator
{
public:
    struct ValidationResult {
        bool isValid;
        QString errorMessage;
        pkt::core::PlayerAction correctedAction;
    };

    PlayerActionValidator() = default;

    /**
     * @brief Validate a betting action (bet/raise)
     * @param amount The amount the player wants to bet/raise
     * @param actionType The type of action (Bet or Raise)
     * @param playerId The player making the action
     * @param playerChips Current player's chip count
     * @param minimumBet Minimum allowed bet for this round
     * @param validActions List of currently valid actions
     * @return ValidationResult with validation status and potential corrections
     */
    ValidationResult validateBettingAction(
        int amount, 
        pkt::core::ActionType actionType,
        int playerId,
        int playerChips,
        int minimumBet,
        const std::vector<pkt::core::ActionType>& validActions
    ) const;

    /**
     * @brief Validate a general action (fold, call, check)
     * @param actionType The type of action
     * @param playerId The player making the action
     * @param validActions List of currently valid actions
     * @return ValidationResult with validation status
     */
    ValidationResult validateGeneralAction(
        pkt::core::ActionType actionType,
        int playerId,
        const std::vector<pkt::core::ActionType>& validActions
    ) const;

    /**
     * @brief Get user-friendly action description
     * @param action The action to describe
     * @return Human-readable action description
     */
    QString getActionDescription(const pkt::core::PlayerAction& action) const;

    /**
     * @brief Check if an action type is in the valid actions list
     * @param actionType The action to check
     * @param validActions List of valid actions
     * @return True if action is valid
     */
    bool isActionValid(pkt::core::ActionType actionType, const std::vector<pkt::core::ActionType>& validActions) const;

private:
    /**
     * @brief Correct betting amount to valid range
     * @param amount Original amount
     * @param minimumBet Minimum allowed bet
     * @param maximumBet Maximum allowed bet (usually player's chips)
     * @return Corrected amount
     */
    int correctBettingAmount(int amount, int minimumBet, int maximumBet) const;
};

} // namespace pkt::ui::qtwidgets