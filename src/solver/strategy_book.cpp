#include "solver/strategy_book.hpp"
#include "core/kuhn_game.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

bool StrategyBook::LoadFromCSV(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs) return false;

    std::string line;
    // skip header
    if (!std::getline(ifs, line)) return false;

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string key, card_str, history;
        if (!std::getline(ss, key, ',')) continue;
        if (!std::getline(ss, card_str, ',')) continue;
        if (!std::getline(ss, history, ',')) continue;

        std::vector<double> probs;
        std::string token;
        while (std::getline(ss, token, ',')) {
            probs.push_back(std::stod(token));
        }

        book_.emplace(key, probs);
    }

    return true;
}

bool StrategyBook::HasStrategy(Card card, const History& history) const
{
    InfoSetKey key = std::string(1, CardToChar(card)) + history;
    return book_.find(key) != book_.end();
}

std::vector<double> StrategyBook::GetStrategy(Card card, const History& history) const
{
    InfoSetKey key = std::string(1, CardToChar(card)) + history;
    auto it = book_.find(key);
    if (it == book_.end()) throw std::logic_error("StrategyBook: unknown infoset");
    return it->second;
}

std::pair<Action, double> StrategyBook::RecommendedAction(Card card, const History& history) const
{
    auto probs = GetStrategy(card, history);
    std::vector<Action> legal = LegalActions(history);
    if (probs.size() != legal.size()) throw std::logic_error("StrategyBook: mismatch action count");

    size_t best = 0;
    for (size_t i = 1; i < probs.size(); ++i) {
        if (probs[i] > probs[best]) best = i;
    }

    return {legal[best], probs[best]};
}
