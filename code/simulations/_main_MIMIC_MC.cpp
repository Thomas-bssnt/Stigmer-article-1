#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

#include "Game.h"
#include "MC_functions.h"
#include "useful_functions.h"

int main()
{
    // Parameters of the program
    const int numberOfGamesInEachStep{50000};
    const MCMethod method{MCMethod::one_pm};
    const std::vector<bool> parametersToChange{true, true, true, true, true, true, true, true};
    const std::string gameType{
        "Group_R2"
        // "Group_R1"
    };
    const std::string initializationType{"fractions"};

    // Parameters of the game
    const int numberOfRounds{20};
    const Rule rule{gameType == "Group_R1" ? Rule::rule_1 : Rule::rule_2};

    // Path of the in and out files
    const std::string pathDataFigures{"../../data_figures/"};
    const std::string pathParameters{pathDataFigures + gameType + "/model/parameters/"};
    const std::string pathObservables{pathDataFigures + gameType + "/exp/observables/"};

    // Get parameters
    std::vector<double> bestParametersVisits{getParameters(pathParameters + "cells.txt")};
    const nlohmann::json parametersStars{nlohmann::json::parse(std::ifstream(pathParameters + "stars.json"))};
    const nlohmann::json parametersPlayersType{nlohmann::json::parse(std::ifstream(pathParameters + "players_type.json"))};

    const int numberOfPlayers{parametersPlayersType["number_of_players"].get<int>()};
    const double tauEvaporation{1e6};

    // Get best parameters and best error
    double bestAverageError{getAverageError(numberOfRounds, numberOfPlayers, rule, tauEvaporation, bestParametersVisits,
                                            parametersPlayersType, initializationType, parametersStars,
                                            numberOfGamesInEachStep, pathObservables)};
    std::cerr << "err =  " << bestAverageError << "\n\n";

    // The MC simulation
    while (true)
    {
        std::time_t t0, t1;
        time(&t0);

        std::vector<double> parameters{bestParametersVisits};
        double averageError{bestAverageError};
        oneMonteCarloStep(method, parametersToChange, numberOfGamesInEachStep, pathObservables, parameters,
                          averageError, numberOfRounds, numberOfPlayers, rule, tauEvaporation, parametersPlayersType,
                          initializationType, parametersStars);
        if (averageError < bestAverageError)
        {
            bestAverageError = averageError;
            bestParametersVisits = parameters;
            writeBestParameters(pathParameters + "cells.txt", bestParametersVisits);
        }

        time(&t1);
        std::cerr << "t = " << std::difftime(t1, t0) << "s, "
                  << "err = " << averageError << "\n\n";
    }

    return 0;
}
