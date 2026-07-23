#include "solver/cfr.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <fstream>
#include <sstream>


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

int CFRSolver::RunIterationsUntilConverged(int max_iters, int check_interval, double eps, int patience, bool use_exploitability)
{
    if (max_iters <= 0 || check_interval <= 0 || patience <= 0 || eps < 0.0)
        throw std::invalid_argument("invalid convergence parameters");

    std::unordered_map<InfoSetKey, std::vector<double>> prev_avg;
    int stable_count = 0;

    for (int iter = 1; iter <= max_iters; ++iter)
    {
        RunIteration();

        if (iter % check_interval != 0)
            continue;

        double max_change = 0.0;

        // compute current average strategies and compare to previous snapshot
        for (const auto& p : infosets_)
        {
            const InfoSetKey& key = p.first;
            const InfoSet& is = p.second;
            auto curr = is.AverageStrategy();

            auto it = prev_avg.find(key);
            if (it == prev_avg.end())
            {
                // treat unseen infoset as moderate change
                for (double v : curr) max_change = std::max(max_change, std::abs(v - 0.0));
            }
            else
            {
                const auto& prev = it->second;
                for (size_t i = 0; i < curr.size() && i < prev.size(); ++i)
                {
                    max_change = std::max(max_change, std::abs(curr[i] - prev[i]));
                }
            }

            prev_avg[key] = curr;
        }

        if (!use_exploitability)
        {
            if (max_change <= eps)
            {
                ++stable_count;
                if (stable_count >= patience)
                    return iter;
            }
            else
            {
                stable_count = 0;
            }
        }
        else
        {
            double ex = Exploitability();
            if (ex <= eps)
            {
                ++stable_count;
                if (stable_count >= patience)
                    return iter;
            }
            else
            {
                stable_count = 0;
            }
        }
    }

    return max_iters;
}

// Best-response recursion: returns payoff to player 1 when `best_player` plays a
// best response and the opponent follows the current average strategy.
static double BestResponseRec(const CFRSolver& solver, const History& history, Card p1_card, Card p2_card, int best_player)
{
    if (IsTerminal(history))
    {
        return Payoff(history, p1_card, p2_card);
    }

    int player = CurrentPlayer(history);
    std::vector<Action> legal_actions = LegalActions(history);

    // If it's the best-response player's turn, choose the action that maximizes
    // (for player 0) or minimizes (for player 1) the payoff to player 1.
    if (player == best_player)
    {
        double best_val = (best_player == 0) ? -1e300 : 1e300;
        for (size_t i = 0; i < legal_actions.size(); ++i)
        {
            History next = ApplyAction(history, legal_actions[i]);
            double child = BestResponseRec(solver, next, p1_card, p2_card, best_player);
            if (best_player == 0)
            {
                if (child > best_val) best_val = child;
            }
            else
            {
                if (child < best_val) best_val = child;
            }
        }
        return best_val;
    }

    // Otherwise the opponent follows the average strategy at this infoset (or uniform).
    InfoSetKey key = MakeInfoSetKey(player == 0 ? p1_card : p2_card, history);
    std::vector<double> strat;
    if (solver.HasInfoSet(player == 0 ? p1_card : p2_card, history))
    {
        strat = solver.GetAverageStrategy(player == 0 ? p1_card : p2_card, history);
    }
    else
    {
        double uniform = 1.0 / legal_actions.size();
        strat.assign(legal_actions.size(), uniform);
    }

    double val = 0.0;
    for (size_t i = 0; i < legal_actions.size(); ++i)
    {
        History next = ApplyAction(history, legal_actions[i]);
        double child = BestResponseRec(solver, next, p1_card, p2_card, best_player);
        val += strat[i] * child;
    }
    return val;
}

double CFRSolver::Exploitability() const
{
    std::vector<std::pair<Card, Card>> deals = {
        {Card::Jack, Card::Queen},
        {Card::Jack, Card::King},
        {Card::Queen, Card::Jack},
        {Card::Queen, Card::King},
        {Card::King, Card::Jack},
        {Card::King, Card::Queen},
    };

    double br1_sum = 0.0;
    double br2_sum = 0.0;
    int count = 0;

    for (const auto& d : deals)
    {
        Card p1 = d.first;
        Card p2 = d.second;
        double v1 = BestResponseRec(*this, "", p1, p2, 0); // value to p1 when p1 best responds
        double v2_p1persp = BestResponseRec(*this, "", p1, p2, 1); // value to p1 when p2 best responds
        br1_sum += v1;
        br2_sum += -v2_p1persp; // convert to player2's perspective
        ++count;
    }

    if (count == 0) return 0.0;

    double br1 = br1_sum / count;
    double br2 = br2_sum / count;

    // NashConv = BR1 + BR2; exploitability = NashConv / 2
    return (br1 + br2) / 2.0;
}