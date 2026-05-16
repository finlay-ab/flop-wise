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

int main()
{
    test_apply_action();
    test_is_terminal();
    test_payoff();
    test_regret_matching();
    test_infoset();
    std::cout << "All tests passed.\n";
    return 0;
}