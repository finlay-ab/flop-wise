#include "solver/infoset.hpp"

#include <algorithm>

std::vector<double> RegretMatching(const std::vector<double> &regrets)
{
    std::vector<double> strategy(regrets.size(), 0.0);

    // running sum of positive regrets
    double sum = 0.0;

    for (size_t i = 0; i < regrets.size(); ++i)
    {
        strategy[i] = std::max(0.0, regrets[i]);
        sum += strategy[i];
    }

    if (sum <= 1e-9)
    {
        double uniform_probability = 1.0 / regrets.size();
        for (size_t i = 0; i < regrets.size(); ++i)
        {
            strategy[i] = uniform_probability;
        }
    }
    else
    {
        for (size_t i = 0; i < regrets.size(); ++i)
        {
            strategy[i] = strategy[i] / sum;
        }
    }

    return strategy;
}

InfoSet::InfoSet(size_t num_actions)
    : cumulative_regrets_(num_actions, 0.0),
      cumulative_strategy_(num_actions, 0.0)
{
}

void InfoSet::AddRegret(size_t action_idx, double regret)
{
    cumulative_regrets_[action_idx] += regret;
    // CFR+: clamp cumulative regrets to non-negative values to speed convergence
    if (cumulative_regrets_[action_idx] < 0.0)
        cumulative_regrets_[action_idx] = 0.0;
}

void InfoSet::AddStrategy(size_t action_idx, double weight)
{
    cumulative_strategy_[action_idx] += weight;
}

std::vector<double> InfoSet::CurrentStrategy() const
{
    return RegretMatching(cumulative_regrets_);
}

std::vector<double> InfoSet::AverageStrategy() const
{
    std::vector<double> strategy(cumulative_strategy_.size(), 0.0);

    double sum = 0.0;

    for (size_t i = 0; i < cumulative_strategy_.size(); ++i)
    {
        sum += cumulative_strategy_[i];
    }

    if (sum <= 1e-9)
    {
        double uniform_probability = 1.0 / cumulative_strategy_.size();
        for (size_t i = 0; i < cumulative_strategy_.size(); ++i)
        {
            strategy[i] = uniform_probability;
        }
    }
    else
    {
        for (size_t i = 0; i < cumulative_strategy_.size(); ++i)
        {
            strategy[i] = cumulative_strategy_[i] / sum;
        }
    }

    return strategy;
}