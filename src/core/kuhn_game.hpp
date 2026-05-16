#pragma once

#include <string>
#include <vector>

enum class Card {
    Jack,
    Queen,
    King
};

enum class Action {
    Check,
    Bet,
    Call,
    Fold
};

using History = std::string;

std::vector<Action> LegalActions(const History& history);
bool IsTerminal(const History& history);
double Payoff(const History& history, Card player1, Card player2);
History ApplyAction(const History& history, Action action);

