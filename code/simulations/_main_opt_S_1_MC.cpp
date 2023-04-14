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
                       const std::vector<double> &parametersVisitsOpt, const std::vector<double> &parametersStarsOpt,
                       const std::vector<double> &parametersVisitsMIMIC, const nlohmann::json &parametersStarsMIMIC,
                       const nlohmann::json &parametersPlayersTypeMIMIC, const int numberOfGames)
{
    // Print the parametersVisitsOpt
    std::cerr << parametersVisitsOpt[0] << " " << parametersVisitsOpt[1] << "\n"
              << parametersVisitsOpt[2] << " " << parametersVisitsOpt[3] << " "
              << parametersVisitsOpt[4] << " " << parametersVisitsOpt[5] << " "
              << parametersVisitsOpt[6] << " " << parametersVisitsOpt[7] << "\n"
              << parametersStarsOpt[0] << " " << parametersStarsOpt[1] << " "
              << parametersStarsOpt[2] << " " << parametersStarsOpt[3] << "\n"
              << parametersStarsOpt[4] << " " << parametersStarsOpt[5] << " "
              << parametersStarsOpt[6] << " " << parametersStarsOpt[7] << "\n";

    // Initialization of the observable
    int S_tot{0};
    int S_exp{0};
    int S_opt{0};

    // Loop on the games
#pragma omp parallel for
    for (int iGame = 0; iGame < numberOfGames; ++iGame)
    {
        // Creation of the game
        Game game(numberOfRounds, numberOfPlayers, rule);

        // Creation of the players
        std::vector<Player> players;
        players.emplace_back(game.getAddress(), parametersVisitsOpt, parametersStarsOpt, "tanh", PlayerType::optimized);
        for (int iPlayer{1}; iPlayer < numberOfPlayers; ++iPlayer)
        {
            double p{randomNumber()};
            if (p < parametersPlayersTypeMIMIC["fractions"][0].get<double>())
            {
                players.emplace_back(game.getAddress(), parametersVisitsMIMIC, parametersStarsMIMIC["def"], PlayerType::defector);
            }
            else if (p < (parametersPlayersTypeMIMIC["fractions"][0].get<double>() +
                          parametersPlayersTypeMIMIC["fractions"][1].get<double>()))
            {
                players.emplace_back(game.getAddress(), parametersVisitsMIMIC, parametersStarsMIMIC["neu"], PlayerType::neutral);
            }
            else
            {
                players.emplace_back(game.getAddress(), parametersVisitsMIMIC, parametersStarsMIMIC["col"], PlayerType::collaborator);
            }
        }

        // Play the game
        for (int iRound{0}; iRound < numberOfRounds; ++iRound)
        {
            for (auto &player : players)
            {
                player.playARound();
            }
        }

        // Update the scores
        for (int iPlayer{0}; iPlayer < numberOfPlayers; ++iPlayer)
        {
            S_tot += game.getScoreOfPlayer(iPlayer);
            if (iPlayer == 0)
            {
                S_opt += game.getScoreOfPlayer(iPlayer);
            }
            else
            {
                S_exp += game.getScoreOfPlayer(iPlayer);
            }
        }
    }

    std::cerr << "S = " << S_tot / static_cast<double>(numberOfGames * numberOfPlayers) << ", "
              << "S_opt = " << S_opt / static_cast<double>(numberOfGames) << ", "
              << "S_exp = " << S_exp / static_cast<double>(numberOfGames * 4) << "\n\n";

    return S_opt / static_cast<double>(numberOfGames);
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
                       std::vector<double> &parametersVisitsOpt, std::vector<double> &parametersStarsOpt,
                       const std::vector<double> &parametersVisitsMIMIC, const nlohmann::json &parametersStarsMIMIC,
                       const nlohmann::json &parametersPlayersTypeMIMIC, double &averageScore,
                       const int numberOfRounds, const int numberOfPlayers, const Rule &rule)
{
    // choice of iParameterToChange and epsilon
    int iParameterToChange;
    do
    {
        iParameterToChange = randomNumber(0, parametersVisitsOpt.size() + parametersStarsOpt.size() - 1);
    } while (!parametersToChange[iParameterToChange]);

    const double epsilon{randomSmallChange(parametersVisitsOpt, iParameterToChange)};

    // + epsilon
    std::vector<double> parametersVisitsPlus{parametersVisitsOpt};
    std::vector<double> parametersStarsPlus{parametersStarsOpt};
    if (iParameterToChange < parametersVisitsOpt.size())
    {
        parametersVisitsPlus[iParameterToChange] += epsilon;
    }
    else
    {
        parametersStarsPlus[iParameterToChange - parametersVisitsOpt.size()] += epsilon;
    }
    double averageScorePlus{getAverageScore(numberOfRounds, numberOfPlayers, rule, parametersVisitsPlus,
                                            parametersStarsPlus, parametersVisitsMIMIC, parametersStarsMIMIC,
                                            parametersPlayersTypeMIMIC, numberOfGames)};
    if (averageScorePlus > averageScore)
    {
        parametersVisitsOpt = parametersVisitsPlus;
        parametersStarsOpt = parametersStarsPlus;
        averageScore = averageScorePlus;
    }

    // - epsilon
    else
    {
        std::vector<double> parametersVisitsMinus{parametersVisitsOpt};
        std::vector<double> parametersStarsMinus{parametersStarsOpt};
        if (iParameterToChange < parametersVisitsOpt.size())
        {
            parametersVisitsMinus[iParameterToChange] -= epsilon;
        }
        else
        {
            parametersStarsMinus[iParameterToChange - parametersVisitsOpt.size()] -= epsilon;
        }
        double averageScoreMinus{getAverageScore(numberOfRounds, numberOfPlayers, rule, parametersVisitsMinus,
                                                 parametersStarsMinus, parametersVisitsMIMIC, parametersStarsMIMIC,
                                                 parametersPlayersTypeMIMIC, numberOfGames)};

        parametersVisitsOpt = parametersVisitsMinus;
        parametersStarsOpt = parametersStarsMinus;
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
    const int numberOfGamesInEachStep{40000};
    const std::vector<bool> parametersToChange{false, false, false, false, false, false, false, false,
                                               false, false, true, false, false, false, false, false};

    // Parameters of the game
    const int numberOfRounds{20};
    const Rule rule{Rule::rule_2};

    // Path of the in and out files
    const std::string pathDataFigures{"../../data_figures/"};

    // Get parameters opt
    const std::string pathParametersOpt{pathDataFigures + "opt_S_1/parameters/"};
    std::vector<double> bestParametersVisits{getParameters(pathParametersOpt + "cells.txt")};
    std::vector<double> bestParametersStars{getParameters(pathParametersOpt + "stars.txt")};

    // Get parameters MIMIC
    const std::string pathParametersMIMIC{pathDataFigures + "Group_R2/model/parameters/"};
    const std::vector<double> parametersVisitsMIMIC{getParameters(pathParametersMIMIC + "cells.txt")};
    const nlohmann::json parametersStarsMIMIC{nlohmann::json::parse(std::ifstream(pathParametersMIMIC + "stars.json"))};
    const nlohmann::json parametersPlayersTypeMIMIC{nlohmann::json::parse(std::ifstream(pathParametersMIMIC + "players_type.json"))};

    const int numberOfPlayers{5};

    // Get best parametersVisitsOpt and best error

    std::time_t t0, t1;
    time(&t0);
    double bestAverageScore{getAverageScore(numberOfRounds, numberOfPlayers, rule, bestParametersVisits,
                                            bestParametersStars, parametersVisitsMIMIC, parametersStarsMIMIC,
                                            parametersPlayersTypeMIMIC, numberOfGamesInEachStep)};
    time(&t1);
    std::cerr << "t = " << std::difftime(t1, t0) << "s\n\n";

    // The MC simulation
    while (true)
    {
        std::vector<double> parametersVisitsOpt{bestParametersVisits};
        std::vector<double> parametersStarsOpt{bestParametersStars};
        double averageScore{bestAverageScore};
        oneMonteCarloStep(parametersToChange, numberOfGamesInEachStep, parametersVisitsOpt, parametersStarsOpt,
                          parametersVisitsMIMIC, parametersStarsMIMIC, parametersPlayersTypeMIMIC, averageScore,
                          numberOfRounds, numberOfPlayers, rule);
        if (averageScore > bestAverageScore)
        {
            bestAverageScore = averageScore;
            bestParametersVisits = parametersVisitsOpt;
            bestParametersStars = parametersStarsOpt;
            writeBestParameters(pathParametersOpt + "cells.txt", bestParametersVisits);
            writeBestParameters(pathParametersOpt + "stars.txt", bestParametersStars);
        }
    }

    return 0;
}
