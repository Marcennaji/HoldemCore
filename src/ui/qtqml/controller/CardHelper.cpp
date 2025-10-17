// HoldemCore — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.

#include "CardHelper.h"
#include <core/engine/cards/CardUtilities.h>
#include <stdexcept>

namespace pkt::ui::qtqml::controller
{

int CardHelper::getCardIndex(const QString& cardString)
{
    if (cardString.isEmpty() || cardString == "??")
    {
        return -1;
    }

    try
    {
        // Use engine's CardUtilities for guaranteed consistency
        return pkt::core::CardUtilities::getCardIndex(cardString.toStdString());
    }
    catch (const std::invalid_argument&)
    {
        // Invalid card string
        return -1;
    }
}

QString CardHelper::getCardString(int cardIndex)
{
    if (cardIndex < 0 || cardIndex >= 52)
    {
        return "Invalid";
    }

    try
    {
        // Use engine's CardUtilities for guaranteed consistency
        const std::string& cardStr = pkt::core::CardUtilities::getCardString(cardIndex);
        return QString::fromStdString(cardStr);
    }
    catch (const std::out_of_range&)
    {
        return "Invalid";
    }
}

bool CardHelper::isValidCard(const QString& cardString)
{
    if (cardString.isEmpty() || cardString == "??")
    {
        return false;
    }

    return pkt::core::CardUtilities::isValidCardString(cardString.toStdString());
}

} // namespace pkt::ui::qtqml::controller
