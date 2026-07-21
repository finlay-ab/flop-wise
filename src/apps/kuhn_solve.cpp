#include <iostream>
#include <string>
#include <cstdlib>
#include "core/kuhn_game.hpp"
#include "solver/cfr.hpp"

int main(int argc, char** argv)
{
    CFRSolver solver;

    // Default save file behavior (compatible with previous behavior)
    std::string savefile = "strategies.csv";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--save" && i + 1 < argc) {
            savefile = argv[i+1];
        }
        if (std::string(argv[i]) == "--no-save") {
            savefile = "";
        }
    }

    // Converge mode: --converge [max_iters] [eps]
    if (argc > 1 && std::string(argv[1]) == "--converge") {
        int max_iters = 100000;
        double eps = 1e-3;
        if (argc > 2) max_iters = std::atoi(argv[2]);
        if (argc > 3) eps = std::atof(argv[3]);

        int check_interval = 100;
        int patience = 3;

        int done = solver.RunIterationsUntilConverged(max_iters, check_interval, eps, patience, true);
        std::cout << "Converged iterations: " << done << " (max " << max_iters << ")" << std::endl;
        std::cout << "Exploitability: " << solver.Exploitability() << std::endl;
    }
    else {
        int iterations = 1000;
        if (argc > 1)
            iterations = std::atoi(argv[1]);

        for (int i = 0; i < iterations; ++i)
            solver.RunIteration();

        std::cout << "Iterations: " << iterations << std::endl;
    }

    std::cout << "InfoSets: " << solver.NumInfoSets() << std::endl;

    if (solver.HasInfoSet(Card::Jack, "")) {
        auto strat = solver.GetAverageStrategy(Card::Jack, "");
        std::cout << "P1 Jack root strategy: ";
        for (double p : strat) std::cout << p << " ";
        std::cout << std::endl;
    } else {
        std::cout << "No root infoset found." << std::endl;
    }

    if (!savefile.empty()) {
        try {
            solver.SaveAverageStrategies(savefile);
            std::cout << "Saved strategies to " << savefile << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Warning: could not save strategies: " << e.what() << std::endl;
        }
    }

    auto best = solver.QueryBestHands(3);
    std::cout << "Top hands (card, EV): ";
    for (auto &p : best) {
        std::cout << CardToChar(p.first) << ":" << p.second << " ";
    }
    std::cout << std::endl;

    return 0;
}