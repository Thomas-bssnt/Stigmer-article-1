#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"

#include "Game.h"
#include "Player.h"
#include "useful_functions.h"

int main()
{
    // Parameters of the program
    const int numberOfGames{10000};
    const std::vector<std::string> gameTypes{"Group_R1", "Group_R2"};
    const std::vector<double> u0s{-1.0, -0.8, -0.6, -0.4, -0.2,
                                  0.0, 0.2, 0.4, 0.6, 0.8,
                                  1.0, 1.2, 1.4, 1.6, 1.8,
                                  2.0, 2.2, 2.4, 2.6, 2.8,
                                  3.0, 3.2, 3.4, 3.6, 3.8,
                                  4.0, 4.2, 4.4, 4.6, 4.8,
                                  5.0, 5.2, 5.4, 5.6, 5.8,
                                  6.0};
    const std::vector<double> u1s{-2.6, -2.4, -2.2,
                                  -2.0, -1.8, -1.6, -1.4, -1.2,
                                  -1.0, -0.8, -0.6, -0.4, -0.2,
                                  0.0, 0.2, 0.4, 0.6, 0.8,
                                  1.0, 1.2, 1.4, 1.6, 1.8,
                                  2.0, 2.2, 2.4};

    // Parameters of the game
    const int numberOfRounds{20};
    const int numberOfPlayers{5};
    const int numberOfValues{100};

    // Path of the in and out files
    const std::string pathDataFigures{"../../data_figures/"};

    for (auto &gameType : gameTypes)
    {
        const std::string pathParameters{pathDataFigures + gameType + "/model/parameters/"};
        const Rule rule{gameType == "Group_R1" ? Rule::rule_1 : Rule::rule_2};
        for (auto &u0 : u0s)
        {
            for (auto &u1 : u1s)
            {
                std::cerr << u0 << " " << u1 << std::endl;

                // Import the optimized parameters
                std::vector<double> parametersVisits{getParameters(pathParameters + "cells.txt")};
                const std::vector<double> parametersStars{u0, u1};

                // Initialization of the observables
                std::vector<std::vector<double>> Q(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> q(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> P(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> p(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> IPR_Q(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> IPR_q(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> IPR_P(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> IPR_p(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> F_Q(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<std::vector<double>> F_P(numberOfRounds, std::vector<double>(numberOfGames, 0));
                std::vector<double> S(numberOfGames * numberOfPlayers, 0);
                std::vector<double> S_team(numberOfGames, 0);

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
                        players.emplace_back(game.getAddress(), parametersVisits, parametersStars, "mns_linear", PlayerType::optimized);
                    }

                    // Compute some properties of the game
                    std::vector<int> sortedValues{game.getValues()};
                    std::sort(sortedValues.begin(), sortedValues.end(), std::greater<>());
                    const int Vmax1{sortedValues[0]};
                    const int Vmax2{sortedValues[1]};
                    const int Vmax3{sortedValues[2]};
                    const int Smax{getScoreMax(rule, numberOfRounds, Vmax1, Vmax2, Vmax3)};

                    // Play the game
                    for (int iRound{0}; iRound < numberOfRounds; ++iRound)
                    {
                        // Do one Round
                        for (auto &player : players)
                        {
                            player.playARound();
                        }

                        // Update some of the observables
                        Q[iRound][iGame] = computeQ(game.getVisitsDistribution(), game.getValues(), Vmax1, Vmax2, Vmax3);
                        q[iRound][iGame] = computeQ(game.getInstantaneousVisitsDistribution(), game.getValues(), Vmax1, Vmax2, Vmax3);
                        P[iRound][iGame] = computeP(game.getStarsDistribution(), game.getValues(), Vmax1);
                        p[iRound][iGame] = computeP(game.getInstantaneousStarsDistribution(), game.getValues(), Vmax1);
                        IPR_Q[iRound][iGame] = computeIPR(game.getVisitsDistribution());
                        IPR_q[iRound][iGame] = computeIPR(game.getInstantaneousVisitsDistribution());
                        IPR_P[iRound][iGame] = computeIPR(game.getStarsDistribution());
                        IPR_p[iRound][iGame] = computeIPR(game.getInstantaneousStarsDistribution());
                        F_Q[iRound][iGame] = computeF(game.getVisitsDistribution(), game.getValues());
                        F_P[iRound][iGame] = computeF(game.getStarsDistribution(), game.getValues());
                    }

                    for (int iPlayer{0}; iPlayer < numberOfPlayers; ++iPlayer)
                    {
                        // Update the scores
                        const double normalizedScore{game.getScoreOfPlayer(iPlayer) / static_cast<double>(Smax)};
                        S[iPlayer + numberOfPlayers * iGame] = normalizedScore;
                        S_team[iGame] += normalizedScore / numberOfPlayers;
                    }
                }

                // Write the observables in different files
                const std::string ruleStr{rule == Rule::rule_1 ? "rule_1/" : "rule_2/"};
                const std::string pathObservables{pathDataFigures + "model_exploration_linear/" + ruleStr +
                                                  "u0=" + std::to_string(static_cast<int>(u0 * 100)) +
                                                  "_u1=" + std::to_string(static_cast<int>(u1 * 100)) + "/"};

                std::filesystem::create_directories(pathObservables);

                saveObservable(pathObservables + "Q", getAverage2dVector(Q));
                saveObservable(pathObservables + "q_", getAverage2dVector(q));
                saveObservable(pathObservables + "P", getAverage2dVector(P));
                saveObservable(pathObservables + "p_", getAverage2dVector(p));
                saveObservable(pathObservables + "IPR_Q", getAverage2dVector(IPR_Q));
                saveObservable(pathObservables + "IPR_q_", getAverage2dVector(IPR_q));
                saveObservable(pathObservables + "IPR_P", getAverage2dVector(IPR_P));
                saveObservable(pathObservables + "IPR_p_", getAverage2dVector(IPR_p));
                saveObservable(pathObservables + "F_Q", getAverage2dVector(F_Q));
                saveObservable(pathObservables + "F_P", getAverage2dVector(F_P));
                saveObservable(pathObservables + "S", getHistogram(S, 50));
                saveObservable(pathObservables + "S_team", getHistogram(S_team, 50));
                saveObservable(pathObservables + "<S>", std::vector<double>{getAverage(S)});
            }
        }
    }
    return 0;
}
