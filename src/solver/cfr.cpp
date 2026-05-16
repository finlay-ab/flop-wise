#include "solver/cfr.hpp"

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
        for (size_t i = 0; i < regrets.size(); ++i)
        {
            strategy[i] = 1.0 / regrets.size();
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