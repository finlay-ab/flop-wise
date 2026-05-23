#include "solver/cfr.hpp"
#include <stdexcept>

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

InfoSetKey MakeInfoSetKey(Card card, const History& history)
{
    return std::string(1, CardToChar(card)) + history;
}

int CurrentPlayer(const History& history)
{
    return history.size() % 2;
}