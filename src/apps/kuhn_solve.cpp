#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include "core/kuhn_game.hpp"
#include "solver/cfr.hpp"

struct Options
{
    bool converge = false;
    int iterations = 1000;
    int max_iters = 100000;
    double eps = 1e-3;
    std::string savefile = "strategies.csv";
};

Options ParseArgs(int argc, char **argv)
{
    Options o;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        auto next = [&]() -> std::string
        {
            if (i + 1 >= argc)
                throw std::invalid_argument("missing value for " + arg);
            return argv[++i];
        };

        if (arg == "--converge")
            o.converge = true;
        else if (arg == "--iters")
            o.iterations = std::stoi(next());
        else if (arg == "--max-iters")
            o.max_iters = std::stoi(next());
        else if (arg == "--eps")
            o.eps = std::stod(next());
        else if (arg == "--save")
            o.savefile = next();
        else if (arg == "--no-save")
            o.savefile.clear();
        else
            throw std::invalid_argument("unknown option: " + arg);
    }
    return o;
}

int main(int argc, char **argv)
{
    Options opts;
    try
    {
        opts = ParseArgs(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    CFRSolver solver;

    if (opts.converge)
    {
        int check_interval = 100;
        int patience = 3;
        int done = solver.RunIterationsUntilConverged(
            opts.max_iters, check_interval, opts.eps, patience, true);
        std::cout << "Converged iterations: " << done
                  << " (max " << opts.max_iters << ")\n";
        std::cout << "Exploitability: " << solver.Exploitability() << "\n";
    }
    else
    {
        for (int i = 0; i < opts.iterations; ++i)
            solver.RunIteration();
        std::cout << "Iterations: " << opts.iterations << "\n";
    }

    std::cout << "InfoSets: " << solver.NumInfoSets() << std::endl;

    if (solver.HasInfoSet(Card::Jack, ""))
    {
        auto strat = solver.GetAverageStrategy(Card::Jack, "");
        std::cout << "P1 Jack root strategy: ";
        for (double p : strat)
            std::cout << p << " ";
        std::cout << std::endl;
    }
    else
    {
        std::cout << "No root infoset found." << std::endl;
    }

    if (!opts.savefile.empty())
    {
        try
        {
            solver.SaveAverageStrategies(opts.savefile);
            std::cout << "Saved strategies to " << opts.savefile << "\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "Warning: could not save strategies: " << e.what() << "\n";
        }
    }

    auto best = solver.QueryBestHands(3);
    std::cout << "Top hands (card, EV): ";
    for (auto &p : best)
    {
        std::cout << CardToChar(p.first) << ":" << p.second << " ";
    }
    std::cout << std::endl;

    return 0;
}