#pragma once

#include <core/engine/EngineDefs.h>
#include <core/player/strategy/CurrentHandContext.h>

#include <iostream>
#include <limits>
#include <string>

namespace pkt::core::ui
{

class UserInputHandler
{
  public:
    UserInputHandler() = default;

    /**
     * Ask the human player for an action via the console.
     * @param ctx The current hand context (position, bets, etc.).
     * @return A PlayerAction representing the chosen move.
     */
    PlayerAction requestAction(const pkt::core::player::CurrentHandContext& ctx)
    {
        PlayerAction action{};
        bool valid = false;

        while (!valid)
        {
            std::cout << "\n=== Your Turn ===\n";
            std::cout << "Position: " << positionToString(ctx.perPlayerContext.myPosition) << "\n";
            std::cout << "Pot size: " << ctx.commonContext.pot << "\n";
            std::cout << "Choose action: (c)all, (r)aise, (f)old, chec(k): ";

            char choice{};
            std::cin >> choice;

            switch (choice)
            {
            case 'c':
            case 'C':
                action.type = ActionType::Call;
                valid = true;
                break;

            case 'r':
            case 'R':
            {
                action.type = ActionType::Raise;
                std::cout << "Enter raise amount: ";
                int amount;
                if (readInt(amount))
                {
                    action.amount = amount;
                    valid = true;
                }
                else
                {
                    std::cout << "Invalid raise amount.\n";
                }
                break;
            }

            case 'f':
            case 'F':
                action.type = ActionType::Fold;
                action.amount = 0;
                valid = true;
                break;

            case 'k':
            case 'K':
                action.type = ActionType::Check;
                action.amount = 0;
                valid = true;
                break;

            default:
                std::cout << "Invalid choice.\n";
                break;
            }
        }
        return action;
    }

  private:
    static bool readInt(int& value)
    {
        std::cin >> value;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    static std::string positionToString(PlayerPosition pos)
    {
        switch (pos)
        {
        case PlayerPosition::SB:
            return "Small Blind";
        case PlayerPosition::BB:
            return "Big Blind";
        case PlayerPosition::UTG:
            return "Under the Gun";
        default:
            return "Unknown";
        }
    }
};

} // namespace pkt::core::ui
