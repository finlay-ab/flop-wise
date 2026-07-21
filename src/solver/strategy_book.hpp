#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "core/kuhn_game.hpp"

using InfoSetKey = std::string;

class StrategyBook {
public:
    bool LoadFromCSV(const std::string& filename);
    bool HasStrategy(Card card, const History& history) const;
    std::vector<double> GetStrategy(Card card, const History& history) const;
    // Returns the recommended action (argmax) and its probability; throws if missing.
    std::pair<Action, double> RecommendedAction(Card card, const History& history) const;

private:
    std::unordered_map<InfoSetKey, std::vector<double>> book_;
};
