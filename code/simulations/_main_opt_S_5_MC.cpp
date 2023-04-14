#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

#include "Game.h"
#include "random.h"
#include "useful_functions.h"

double getAverageScore(const int numberOfRounds, const int numberOfPlayers, const Rule &rule,
                       const std::vector<double> &parametersVisits, const std::vector<double> &parametersStars,
                       const int numberOfGames)
{
    // Print the parametersVisits
    std::cerr << parametersVisits[0] << " " << parametersVisits[1] << "\n"
              << parametersVisits[2] << " " << parametersVisits[3] << " "
              << parametersVisits[4] << " " << parametersVisits[5] << " "
              << parametersVisits[6] << " " << parametersVisits[7] << "\n"
              << parametersStars[0] << " " << parametersStars[1] << " "
              << parametersStars[2] << " " << parametersStars[3] << "\n"
              << parametersStars[4] << " " << parametersStars[5] << " "
              << parametersStars[6] << " " << parametersStars[7] << "\n";

    // Initialization of the observable
    int S_tot{0};

    // Loop on the games
#pragma omp parallel for
    for (int iGame = 0; iGame < numberOfGames; ++iGame)
    {
        // Creation of the game
        Game game(numberOfRounds, numberOfPlayers, rule);

        // Creation of the players
        std::vector<Player> players;
        for (int iPlayer{0}; iPlayer < numberOfPlayers; ++iPlayer)
        {
            players.emplace_back(game.getAddress(), parametersVisits, parametersStars, "tanh", PlayerType::optimized);
        }

        // Play the game
        for (int iRound{0}; iRound < numberOfRounds; ++iRound)
        {
            for (auto &player : players)
            {
                player.playARound();
            }
        }

        // Update the total score
        for (int iPlayer{0}; iPlayer < numberOfPlayers; ++iPlayer)
        {
            S_tot += game.getScoreOfPlayer(iPlayer);
        }
    }

    std::cerr << "S = " << S_tot / static_cast<double>(numberOfGames * numberOfPlayers) << "\n\n";

    return S_tot / static_cast<double>(numberOfGames * numberOfPlayers);
}

double randomSmallChange(const std::vector<double> &parameters, const int iParameterToChange)
{
    double epsilon{0.};
    switch (iParameterToChange)
    {
    case 0:
        do
        {
            epsilon = randomNumber(0., 1.) * 0.1;
        } while (parameters[0] - epsilon < 0 ||
                 parameters[0] + epsilon < 0 ||
                 parameters[0] - epsilon > 1 ||
                 parameters[0] + epsilon > 1);
        break;
    case 2:
    case 4:
    case 6:
    case 10:
    case 14:
        epsilon = randomNumber(0., 1.) * 10;
        break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 9:
    case 11:
    case 12:
    case 13:
    case 15:
        epsilon = randomNumber(0., 1.) * 0.2;
        break;
    default:
        std::cerr << "The parameter " << iParameterToChange << " is not implemented.\n";
        break;
    }
    return epsilon;
}

void oneMonteCarloStep(const std::vector<bool> &parametersToChange, const int numberOfGames,
                       std::vector<double> &parametersVisits, std::vector<double> &parametersStars, double &averageScore,
                       const int numberOfRounds, const int numberOfPlayers, const Rule &rule)
{
    // choice of iParameterToChange and epsilon
    int iParameterToChange;
    do
    {
        iParameterToChange = randomNumber(0, parametersVisits.size() + parametersStars.size() - 1);
    } while (!parametersToChange[iParameterToChange]);

    const double epsilon{randomSmallChange(parametersVisits, iParameterToChange)};

    // + epsilon
    std::vector<double> parametersVisitsPlus{parametersVisits};
    std::vector<double> parametersStarsPlus{parametersStars};
    if (iParameterToChange < parametersVisits.size())
    {
        parametersVisitsPlus[iParameterToChange] += epsilon;
    }
    else
    {
        parametersStarsPlus[iParameterToChange - parametersVisits.size()] += epsilon;
    }
    double averageScorePlus{getAverageScore(numberOfRounds, numberOfPlayers, rule, parametersVisitsPlus,
                                            parametersStarsPlus, numberOfGames)};
    if (averageScorePlus > averageScore)
    {
        parametersVisits = parametersVisitsPlus;
        parametersStars = parametersStarsPlus;
        averageScore = averageScorePlus;
    }

    // - epsilon
    else
    {
        std::vector<double> parametersVisitsMinus{parametersVisits};
        std::vector<double> parametersStarsMinus{parametersStars};
        if (iParameterToChange < parametersVisits.size())
        {
            parametersVisitsMinus[iParameterToChange] -= epsilon;
        }
        else
        {
            parametersStarsMinus[iParameterToChange - parametersVisits.size()] -= epsilon;
        }
        double averageScoreMinus{getAverageScore(numberOfRounds, numberOfPlayers, rule, parametersVisitsMinus,
                                                 parametersStarsMinus, numberOfGames)};

        parametersVisits = parametersVisitsMinus;
        parametersStars = parametersStarsMinus;
        averageScore = averageScoreMinus;
    }
}

void writeBestParameters(const std::string &filePath, const std::vector<double> &bestParameters)
{
    std::ofstream outFile(filePath, std::ios::app);
    if (outFile.is_open())
    {
        outFile << bestParameters[0];
        for (int iParameter{1}; iParameter < bestParameters.size(); ++iParameter)
        {
            outFile << " " << bestParameters[iParameter];
        }
        outFile << "\n";
    }
    else
    {
        std::cerr << "The file " << filePath << " could not be opened.\n";
    }
}

int main()
{
    // Parameters of the program
    const int numberOfGamesInEachStep{50000};
    const std::vector<bool> parametersToChange{false, true, true, true, true, true, true, true,
                                               false, true, true, true, false, true, true, true};

    // Parameters of the game
    const int numberOfRounds{20};
    const Rule rule{Rule::rule_2};

    // Path of the in and out files
    const std::string pathDataFigures{"../../data_figures/"};
    const std::string pathParameters{pathDataFigures + "opt_S_5/parameters/"};

    // Get parameters
    std::vector<double> bestParametersVisits{getParameters(pathParameters + "cells.txt")};
    std::vector<double> bestParametersStars{getParameters(pathParameters + "stars.txt")};

    const int numberOfPlayers{5};

    // Get best parameters and best error

    std::time_t t0, t1;
    time(&t0);
    double bestAverageScore{getAverageScore(numberOfRounds, numberOfPlayers, rule, bestParametersVisits,
                                            bestParametersStars, numberOfGamesInEachStep)};
    time(&t1);
    std::cerr << "t = " << std::difftime(t1, t0) << "s\n\n";

    // The MC simulation
    while (true)
    {
        std::vector<double> parametersVisits{bestParametersVisits};
        std::vector<double> parametersStars{bestParametersStars};
        double averageScore{bestAverageScore};
        oneMonteCarloStep(parametersToChange, numberOfGamesInEachStep, parametersVisits, parametersStars,
                          averageScore, numberOfRounds, numberOfPlayers, rule);
        if (averageScore > bestAverageScore)
        {
            bestAverageScore = averageScore;
            bestParametersVisits = parametersVisits;
            bestParametersStars = parametersStars;
            writeBestParameters(pathParameters + "cells.txt", bestParametersVisits);
            writeBestParameters(pathParameters + "stars.txt", bestParametersStars);
        }
    }

    return 0;
}
