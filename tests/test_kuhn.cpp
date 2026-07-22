#include "core/kuhn_game.hpp"
#include "solver/infoset.hpp"
#include "solver/cfr.hpp"
#include "check.hpp"

#include <iostream>
#include <cmath>

bool approx_eq(double a, double b, double eps = 1e-6)
{
    return std::abs(a - b) < eps;
}

void test_cfr_terminal_histories()
{
    CFRSolver solver;

    CHECK(approx_eq(solver.CFR("xx", Card::King, Card::Jack, 1.0, 1.0), 1.0));
    CHECK(approx_eq(solver.CFR("bf", Card::Jack, Card::Queen, 1.0, 1.0), 1.0));
    CHECK(approx_eq(solver.CFR("bc", Card::King, Card::Jack, 1.0, 1.0), 2.0));
    CHECK(approx_eq(solver.CFR("xbf", Card::King, Card::Jack, 1.0, 1.0), -1.0));
    CHECK(approx_eq(solver.CFR("xbc", Card::King, Card::Jack, 1.0, 1.0), 2.0));
}

void test_cfr_run_iteration()
{
    CFRSolver solver;
    CHECK(solver.NumInfoSets() == 0);

    solver.RunIteration();

    CHECK(solver.NumInfoSets() > 0);

    auto root_strategy = solver.GetAverageStrategy(Card::Jack, "");
    CHECK(root_strategy.size() == 2);
    CHECK(approx_eq(root_strategy[0] + root_strategy[1], 1.0));
}

void test_cfr_training()
{
    CFRSolver solver;
    const int iterations = 1000;
    for (int i = 0; i < iterations; ++i)
        solver.RunIteration();

    CHECK(solver.NumInfoSets() > 0);
    CHECK(solver.HasInfoSet(Card::Jack, ""));

    auto strat = solver.GetAverageStrategy(Card::Jack, "");
    CHECK(strat.size() == 2);
    CHECK(approx_eq(strat[0] + strat[1], 1.0));
    for (double p : strat) {
        CHECK(p >= 0.0 && p <= 1.0);
    }
}

// core/kuhn_game
void test_apply_action()
{
    History h = "";
    h = ApplyAction(h, Action::Check);
    CHECK(h == "x");
    h = ApplyAction(h, Action::Bet);
    CHECK(h == "xb");
}
void test_is_terminal()
{
    CHECK(!IsTerminal(""));
    CHECK(!IsTerminal("x"));
    CHECK(IsTerminal("xx"));
    CHECK(IsTerminal("bf"));
    CHECK(IsTerminal("xbc"));
}
void test_payoff()
{
    CHECK(Payoff("xx", Card::King, Card::Jack) == 1.0);
    CHECK(Payoff("xx", Card::Jack, Card::King) == -1.0);
    CHECK(Payoff("bf", Card::Jack, Card::King) == 1.0);
    CHECK(Payoff("bc", Card::King, Card::Jack) == 2.0);
    CHECK(Payoff("xbf", Card::King, Card::Jack) == -1.0);
    CHECK(Payoff("bf", Card::Jack, Card::Queen) == 1.0);
    CHECK(Payoff("bf", Card::Queen, Card::Jack) == 1.0); 
    CHECK(Payoff("bf", Card::Jack, Card::Jack) == 1.0);

}

// solver/cfr
void test_regret_matching()
{
    std::vector<double> regrets1 = {5.0, 3.0, -2.0};
    auto strat1 = RegretMatching(regrets1);
    CHECK(approx_eq(strat1[0], 0.625));
    CHECK(approx_eq(strat1[1], 0.375));
    CHECK(approx_eq(strat1[2], 0.0));

    std::vector<double> regrets2 = {-1.0, -2.0, -3.0};
    auto strat2 = RegretMatching(regrets2);
    CHECK(approx_eq(strat2[0], 1.0/3.0));
    CHECK(approx_eq(strat2[1], 1.0/3.0));
    CHECK(approx_eq(strat2[2], 1.0/3.0));
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
    CHECK(approx_eq(current[0], 5.0/8.0));
    CHECK(approx_eq(current[1], 3.0/8.0));

    auto avg = is.AverageStrategy();
    CHECK(approx_eq(avg[0], 0.5));
    CHECK(approx_eq(avg[1], 0.5));
}

void test_card_to_char() {
    CHECK(CardToChar(Card::Jack) == 'J');
    CHECK(CardToChar(Card::Queen) == 'Q');
    CHECK(CardToChar(Card::King) == 'K');
}

void test_make_infoset_key() {
    CHECK(MakeInfoSetKey(Card::Jack, "") == "J");
    CHECK(MakeInfoSetKey(Card::Queen, "x") == "Qx");
    CHECK(MakeInfoSetKey(Card::King, "xb") == "Kxb");
    CHECK(MakeInfoSetKey(Card::Jack, "b") == "Jb");
}

void test_current_player() {
    CHECK(CurrentPlayer("") == 0);    // P1 acts first
    CHECK(CurrentPlayer("x") == 1);   // P2 responds to check
    CHECK(CurrentPlayer("b") == 1);   // P2 responds to bet
    CHECK(CurrentPlayer("xb") == 0);  // P1 responds to P2's bet
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
    // convergence early-stop smoke test
    {
        CFRSolver s;
        int done = s.RunIterationsUntilConverged(100, 1, 2.0, 1);
        CHECK(done > 0 && done <= 100);
    }
    // exploitability smoke test
    {
        CFRSolver s;
        s.RunIteration();
        double ex = s.Exploitability();
        CHECK(ex >= 0.0);
    }
    std::cout << "All tests passed.\n";
    return 0;
}