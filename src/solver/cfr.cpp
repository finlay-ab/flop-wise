#include "solver/cfr.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <map>

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

InfoSet& CFRSolver::GetOrCreateInfoSet(const InfoSetKey& key, size_t num_actions)
{
    auto it = infosets_.find(key);
    if (it == infosets_.end())
    {
        it = infosets_.emplace(key, InfoSet(num_actions)).first;
    }

    return it->second;
}

double CFRSolver::CFR(const History& history, Card p1_card, Card p2_card, double reach_p1, double reach_p2)
{
    if (IsTerminal(history))
    {
        return Payoff(history, p1_card, p2_card);
    }

    int player = CurrentPlayer(history);

    std::vector<Action> legal_actions = LegalActions(history);
    InfoSetKey key = MakeInfoSetKey(player == 0 ? p1_card : p2_card, history);
    InfoSet& infoset = GetOrCreateInfoSet(key, legal_actions.size());
    std::vector<double> strategy = infoset.CurrentStrategy();

    std::vector<double> action_values(legal_actions.size(), 0.0);
    double node_value = 0.0;

    for (size_t i = 0; i < legal_actions.size(); ++i)
    {
        History next_history = ApplyAction(history, legal_actions[i]);

        if (player == 0)
        {
            action_values[i] = CFR(next_history, p1_card, p2_card, reach_p1 * strategy[i], reach_p2);
        }
        else
        {
            action_values[i] = CFR(next_history, p1_card, p2_card, reach_p1, reach_p2 * strategy[i]);
        }

        node_value += strategy[i] * action_values[i];
    }

    for (size_t i = 0; i < legal_actions.size(); ++i)
    {
        if (player == 0)
        {
            infoset.AddRegret(i, reach_p2 * (action_values[i] - node_value));
            infoset.AddStrategy(i, reach_p1 * strategy[i]);
        }
        else
        {
            infoset.AddRegret(i, reach_p1 * (node_value - action_values[i]));
            infoset.AddStrategy(i, reach_p2 * strategy[i]);
        }
    }

    return node_value;
}

void CFRSolver::RunIteration()
{
    const std::vector<std::pair<Card, Card>> deals = {
        {Card::Jack, Card::Queen},
        {Card::Jack, Card::King},
        {Card::Queen, Card::Jack},
        {Card::Queen, Card::King},
        {Card::King, Card::Jack},
        {Card::King, Card::Queen},
    };

    for (const auto& deal : deals)
    {
        CFR("", deal.first, deal.second, 1.0, 1.0);
    }
}

std::vector<double> CFRSolver::GetAverageStrategy(Card card, const History& history) const
{
    InfoSetKey key = MakeInfoSetKey(card, history);
    auto it = infosets_.find(key);
    if (it == infosets_.end())
    {
        throw std::logic_error("GetAverageStrategy: unknown infoset");
    }

    return it->second.AverageStrategy();
}

bool CFRSolver::HasInfoSet(Card card, const History& history) const
{
    InfoSetKey key = MakeInfoSetKey(card, history);
    return infosets_.find(key) != infosets_.end();
}

double CFRSolver::EvaluateAveragePolicy(const History& history, Card p1_card, Card p2_card) const
{
    if (IsTerminal(history))
    {
        return Payoff(history, p1_card, p2_card);
    }

    int player = CurrentPlayer(history);
    std::vector<Action> legal_actions = LegalActions(history);

    InfoSetKey key = MakeInfoSetKey(player == 0 ? p1_card : p2_card, history);
    auto it = infosets_.find(key);

    std::vector<double> strategy;
    if (it == infosets_.end())
    {
        double uniform = 1.0 / legal_actions.size();
        strategy.assign(legal_actions.size(), uniform);
    }
    else
    {
        strategy = it->second.AverageStrategy();
    }

    double node_value = 0.0;
    for (size_t i = 0; i < legal_actions.size(); ++i)
    {
        History next_history = ApplyAction(history, legal_actions[i]);
        double child = EvaluateAveragePolicy(next_history, p1_card, p2_card);
        node_value += strategy[i] * child;
    }

    return node_value;
}

double EvalHandExpected(const CFRSolver& solver, Card card)
{
    // average over opponent cards uniformly
    std::vector<Card> all = {Card::Jack, Card::Queen, Card::King};
    double sum = 0.0;
    int count = 0;
    for (Card c2 : all)
    {
        if (c2 == card) continue;
        sum += solver.EvaluateAveragePolicy("", card, c2);
        ++count;
    }
    return sum / std::max(1, count);
}

void CFRSolver::SaveAverageStrategies(const std::string& filename) const
{
    std::ofstream ofs(filename);
    if (!ofs)
        throw std::runtime_error("unable to open file for writing: " + filename);

    ofs << "key,card,history,prob0,prob1\n";
    for (const auto& p : infosets_)
    {
        const InfoSetKey& key = p.first;
        const InfoSet& is = p.second;
        auto avg = is.AverageStrategy();
        ofs << key << "," << key[0] << "," << key.substr(1) << ",";
        for (size_t i = 0; i < avg.size(); ++i)
        {
            if (i) ofs << ",";
            ofs << avg[i];
        }
        ofs << "\n";
    }
}

std::vector<std::pair<Card, double>> CFRSolver::QueryBestHands(size_t top_n) const
{
    std::vector<std::pair<Card, double>> vals;
    std::vector<Card> all = {Card::Jack, Card::Queen, Card::King};
    for (Card c : all)
    {
        double ev = EvalHandExpected(*this, c);
        vals.emplace_back(c, ev);
    }

    std::sort(vals.begin(), vals.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
    if (top_n < vals.size()) vals.resize(top_n);
    return vals;
}