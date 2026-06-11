#include "core/kuhn_game.hpp"
#include "solver/infoset.hpp"
#include "solver/cfr.hpp"

#include <cassert>
#include <iostream>
#include <cmath>

bool approx_eq(double a, double b, double eps = 1e-6)
{
    return std::abs(a - b) < eps;
}

void test_cfr_terminal_histories()
{
    CFRSolver solver;

    assert(approx_eq(solver.CFR("xx", Card::King, Card::Jack, 1.0, 1.0), 1.0));
    assert(approx_eq(solver.CFR("bf", Card::Jack, Card::Queen, 1.0, 1.0), 1.0));
    assert(approx_eq(solver.CFR("bc", Card::King, Card::Jack, 1.0, 1.0), 2.0));
    assert(approx_eq(solver.CFR("xbf", Card::King, Card::Jack, 1.0, 1.0), -1.0));
    assert(approx_eq(solver.CFR("xbc", Card::King, Card::Jack, 1.0, 1.0), 2.0));
}

void test_cfr_run_iteration()
{
    CFRSolver solver;
    assert(solver.NumInfoSets() == 0);

    solver.RunIteration();

    assert(solver.NumInfoSets() > 0);

    auto root_strategy = solver.GetAverageStrategy(Card::Jack, "");
    assert(root_strategy.size() == 2);
    assert(approx_eq(root_strategy[0] + root_strategy[1], 1.0));
}

void test_cfr_training()
{
    CFRSolver solver;
    const int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
        solver.RunIteration();

    assert(solver.NumInfoSets() > 0);
    assert(solver.HasInfoSet(Card::Jack, ""));

    auto strat = solver.GetAverageStrategy(Card::Jack, "");
    assert(strat.size() == 2);
    assert(approx_eq(strat[0] + strat[1], 1.0));
    for (double p : strat) {
        assert(p >= 0.0 && p <= 1.0);
    }
}

// core/kuhn_game
void test_apply_action()
{
    History h = "";
    h = ApplyAction(h, Action::Check);
    assert(h == "x");
    h = ApplyAction(h, Action::Bet);
    assert(h == "xb");
}
void test_is_terminal()
{
    assert(!IsTerminal(""));
    assert(!IsTerminal("x"));
    assert(IsTerminal("xx"));
    assert(IsTerminal("bf"));
    assert(IsTerminal("xbc"));
}
void test_payoff()
{
    assert(Payoff("xx", Card::King, Card::Jack) == 1.0);
    assert(Payoff("xx", Card::Jack, Card::King) == -1.0);
    assert(Payoff("bf", Card::Jack, Card::King) == 1.0);
    assert(Payoff("bc", Card::King, Card::Jack) == 2.0);
    assert(Payoff("xbf", Card::King, Card::Jack) == -1.0);
    assert(Payoff("bf", Card::Jack, Card::Queen) == 1.0);
    assert(Payoff("bf", Card::Queen, Card::Jack) == 1.0); 
    assert(Payoff("bf", Card::Jack, Card::Jack) == 1.0);

}

// solver/cfr
void test_regret_matching()
{
    std::vector<double> regrets1 = {5.0, 3.0, -2.0};
    auto strat1 = RegretMatching(regrets1);
    assert(approx_eq(strat1[0], 0.625));
    assert(approx_eq(strat1[1], 0.375));
    assert(approx_eq(strat1[2], 0.0));

    std::vector<double> regrets2 = {-1.0, -2.0, -3.0};
    auto strat2 = RegretMatching(regrets2);
    assert(approx_eq(strat2[0], 1.0/3.0));
    assert(approx_eq(strat2[1], 1.0/3.0));
    assert(approx_eq(strat2[2], 1.0/3.0));
}

// solver/infoset
void test_infoset()
{
    InfoSet is(2);
    is.AddRegret(0, 5.0);
    is.AddRegret(1, 3.0);
    is.AddStrategy(0, 0.5);
    is.AddStrategy(1, 0.5);

    auto current = is.CurrentStrategy();
    assert(approx_eq(current[0], 5.0/8.0));
    assert(approx_eq(current[1], 3.0/8.0));

    auto avg = is.AverageStrategy();
    assert(approx_eq(avg[0], 0.5));
    assert(approx_eq(avg[1], 0.5));
}

void test_card_to_char() {
    assert(CardToChar(Card::Jack) == 'J');
    assert(CardToChar(Card::Queen) == 'Q');
    assert(CardToChar(Card::King) == 'K');
}

void test_make_infoset_key() {
    assert(MakeInfoSetKey(Card::Jack, "") == "J");
    assert(MakeInfoSetKey(Card::Queen, "x") == "Qx");
    assert(MakeInfoSetKey(Card::King, "xb") == "Kxb");
    assert(MakeInfoSetKey(Card::Jack, "b") == "Jb");
}

void test_current_player() {
    assert(CurrentPlayer("") == 0);    // P1 acts first
    assert(CurrentPlayer("x") == 1);   // P2 responds to check
    assert(CurrentPlayer("b") == 1);   // P2 responds to bet
    assert(CurrentPlayer("xb") == 0);  // P1 responds to P2's bet
}

int main()
{
    test_apply_action();
    test_is_terminal();
    test_payoff();
    test_cfr_terminal_histories();
    test_cfr_run_iteration();
    test_regret_matching();
    test_infoset();
    test_make_infoset_key();
    test_current_player();
    std::cout << "All tests passed.\n";
    return 0;
}