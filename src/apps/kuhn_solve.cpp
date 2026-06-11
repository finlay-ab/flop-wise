#include <iostream>
#include "core/kuhn_game.hpp"
#include "solver/cfr.hpp"

int main(int argc, char** argv)
{
    int iterations = 1000;
    if (argc > 1)
        iterations = std::atoi(argv[1]);

    CFRSolver solver;
    for (int i = 0; i < iterations; ++i)
        solver.RunIteration();

    std::cout << "Iterations: " << iterations << std::endl;
    std::cout << "InfoSets: " << solver.NumInfoSets() << std::endl;

    if (solver.HasInfoSet(Card::Jack, "")) {
        auto strat = solver.GetAverageStrategy(Card::Jack, "");
        std::cout << "P1 Jack root strategy: ";
        for (double p : strat) std::cout << p << " ";
        std::cout << std::endl;
    } else {
        std::cout << "No root infoset found." << std::endl;
    }

    // optionally save strategies
    std::string savefile = "strategies.csv";
    if (argc > 2 && std::string(argv[2]) == "--save" && argc > 3)
        savefile = argv[3];

    try {
        solver.SaveAverageStrategies(savefile);
        std::cout << "Saved strategies to " << savefile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: could not save strategies: " << e.what() << std::endl;
    }

    auto best = solver.QueryBestHands(3);
    std::cout << "Top hands (card, EV): ";
    for (auto &p : best) {
        std::cout << CardToChar(p.first) << ":" << p.second << " ";
    }
    std::cout << std::endl;

    return 0;
}