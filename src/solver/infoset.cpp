#include "solver/infoset.hpp"
#include "solver/cfr.hpp"

InfoSet::InfoSet(size_t num_actions)
    : cumulative_regrets_(num_actions, 0.0),
      cumulative_strategy_(num_actions, 0.0)
{
    // used prefer list for efficency
}

void InfoSet::AddRegret(size_t action_idx, double regret)
{
    cumulative_regrets_[action_idx] += regret;
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

    // running sum of positive regrets
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
