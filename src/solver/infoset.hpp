#pragma once

#include <vector>

std::vector<double> RegretMatching(const std::vector<double>& regrets);

class InfoSet {
public:
    InfoSet(size_t num_actions);

    std::vector<double> CurrentStrategy() const;
    std::vector<double> AverageStrategy() const;
    void AddRegret(size_t action_idx, double regret);
    void AddStrategy(size_t action_idx, double weight);

private:
    std::vector<double> cumulative_regrets_;
    std::vector<double> cumulative_strategy_;
};