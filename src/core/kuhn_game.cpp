#include "core/kuhn_game.hpp"

#include <stdexcept>

History ApplyAction(const History &history, Action action)
{
    History newHistory;
    switch (action)
    {
    case Action::Check:
        newHistory = history + 'x';
        break;
    case Action::Bet:
        newHistory = history + 'b';
        break;
    case Action::Call:
        newHistory = history + 'c';
        break;
    case Action::Fold:
        newHistory = history + 'f';
        break;
    default:
        throw std::logic_error("ApplyAction: unknown Action enum value");
        break;
    }

    return newHistory;
}

bool IsTerminal(const History &history)
{
    if (history.empty())
    {
        return false;
    }

    switch (history.back())
    {
    case 'f':
    case 'c':
        return true;
    case 'x':
        return history.size() >= 2 && history[history.size() - 2] == 'x';
    default:
        break;
    }

    return false;
}

std::vector<Action> LegalActions(const History &history)
{
    if (IsTerminal(history))
    {
        throw std::logic_error("LegalActions: called on terminal history: \"" + history + "\"");
    }

    if (history.empty() || history == "x")
    {
        return {Action::Check, Action::Bet};
    }

    if (history == "b" || history == "xb")
    {
        return {Action::Call, Action::Fold};
    }

    throw std::logic_error("LegalActions: called on invalid history: \"" + history + "\"");
}

// player 1s payoff
double Payoff(const History &history, Card player1, Card player2)
{
    if (!IsTerminal(history))
    {
        throw std::logic_error("Payoff: called on non terminal history: \"" + history + "\"");
    }

    switch (history.back())
    {
    case 'f':
        return (history.length() % 2 == 0) ? +1.0 : -1.0;
    case 'x':
        return static_cast<int>(player1) > static_cast<int>(player2) ? 1.0 : -1.0;
    case 'c':
        return static_cast<int>(player1) > static_cast<int>(player2) ? 2.0 : -2.0;
    default:
        break;
    }

    throw std::logic_error("Payoff: called on an invalid history: \"" + history + "\"");
}

char CardToChar(Card card)
{
    switch (card)
    {
    case Card::Jack:
        return 'J';
    case Card::Queen:
        return 'Q';
    case Card::King:
        return 'K';
    default:
        break;
    }

    throw std::logic_error("CardToChar: called on invalid card.");
}