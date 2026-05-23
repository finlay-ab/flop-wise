#pragma once

#include <vector>
#include <numeric>
#include "core/kuhn_game.hpp"
#include <stdexcept>

using InfoSetKey = std::string;

std::vector<double> RegretMatching(const std::vector<double>& regrets);

char CardToChar(Card card);
InfoSetKey MakeInfoSetKey(Card card, const History& history);
int CurrentPlayer(const History& history);
