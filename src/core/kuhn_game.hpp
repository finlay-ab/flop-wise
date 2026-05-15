#pragma once

#include <string>

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