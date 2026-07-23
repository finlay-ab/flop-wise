#pragma once

#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <utility>
#include <string>
#include "core/kuhn_game.hpp"
#include "solver/infoset.hpp"

using InfoSetKey = std::string;

InfoSetKey MakeInfoSetKey(Card card, const History& history);
int CurrentPlayer(const History& history);

class CFRSolver {
public:
    size_t NumInfoSets() const { return infosets_.size(); }
    void RunIteration();
    std::vector<double> GetAverageStrategy(Card card, const History& history) const;
    bool HasInfoSet(Card card, const History& history) const;
    void SaveAverageStrategies(const std::string& filename) const;
    std::vector<std::pair<Card, double>> QueryBestHands(size_t top_n = 3) const;
    int RunIterationsUntilConverged(int max_iters, int check_interval, double eps, int patience = 3, bool use_exploitability = false);
    double Exploitability() const;
    double EvaluateAveragePolicy(const History& history, Card p1_card, Card p2_card) const;

private:
    std::unordered_map<InfoSetKey, InfoSet> infosets_;

    // allows for testing of terminal without making it public
    friend void test_cfr_terminal_histories();

    InfoSet& GetOrCreateInfoSet(const InfoSetKey& key, size_t num_actions);
    double CFR(const History& history, Card p1_card, Card p2_card, double reach_p1, double reach_p2);
};