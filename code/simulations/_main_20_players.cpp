#include <algorithm>
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

    const std::string initializationType{"fractions"};

    for (auto &gameType : gameTypes)
    {
        // Parameters of the game
        const int numberOfRounds{20};
        const Rule rule{gameType == "Group_R1" ? Rule::rule_1 : Rule::rule_2};

        // Path of the in and out files
        const std::string pathDataFigures{"../../data_figures/"};
        const std::string pathParameters{pathDataFigures + gameType + "/model/parameters/"};

        // Import the optimized parameters
        const std::vector<double> parametersVisits{getParameters(pathParameters + "cells.txt")};
        const nlohmann::json parametersStars{nlohmann::json::parse(std::ifstream(pathParameters + "stars.json"))};
        const nlohmann::json parametersPlayersType{nlohmann::json::parse(std::ifstream(pathParameters + "players_type.json"))};

        const int numberOfPlayers{20};

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
        std::vector<std::vector<double>> B1(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> B2(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> B3(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> V1(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> V2(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> V3(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> VB1(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> VB2(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<std::vector<double>> VB3(numberOfRounds, std::vector<double>(numberOfGames, 0));
        std::vector<double> S(numberOfGames * numberOfPlayers, 0);
        std::vector<double> S_team(numberOfGames, 0);
        std::vector<int> rankCollaborator(numberOfPlayers, 0);
        std::vector<int> rankNeutral(numberOfPlayers, 0);
        std::vector<int> rankDefector(numberOfPlayers, 0);
        std::vector<int> find99(numberOfRounds, 0);
        std::vector<int> find80(numberOfRounds, 0);
        std::vector<int> find70(numberOfRounds, 0);

        // Loop on the games
#pragma omp parallel for
        for (int iGame = 0; iGame < numberOfGames; ++iGame)
        {
            // Creation of the game
            Game game(numberOfRounds, 3, numberOfPlayers, 900, 5, rule, MapType::random, 1.e6);

            // Creation of the players
            std::vector<Player>
                players{initializePlayers(parametersPlayersType, numberOfPlayers, game, parametersVisits, parametersStars)};

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

                    // Update found cells
                    const std::vector<bool> foundBestCells{player.getFoundCells()};
                    for (int iMap{0}; iMap < 4; ++iMap)
                    {
                        find99[iRound] += foundBestCells[iMap * 225 + 0];
                        find80[iRound] += foundBestCells[iMap * 225 + 1];
                        find80[iRound] += foundBestCells[iMap * 225 + 2];
                        find80[iRound] += foundBestCells[iMap * 225 + 3];
                        find80[iRound] += foundBestCells[iMap * 225 + 4];
                        find70[iRound] += foundBestCells[iMap * 225 + 5];
                        find70[iRound] += foundBestCells[iMap * 225 + 6];
                        find70[iRound] += foundBestCells[iMap * 225 + 7];
                        find70[iRound] += foundBestCells[iMap * 225 + 8];
                    }
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
                // Update the values of B1, B2, B3
                const std::vector<std::vector<int>> playBestCellsRound{players[iPlayer].getPlayBestCellsRound()};
                for (int iRound{0}; iRound < numberOfRounds; ++iRound)
                {
                    B1[iRound][iGame] += playBestCellsRound[0][iRound];
                    B2[iRound][iGame] += playBestCellsRound[1][iRound];
                    B3[iRound][iGame] += playBestCellsRound[2][iRound];
                }
                // Update the values of V1, V2, V3
                const std::vector<std::vector<int>> valueBestCells{players[iPlayer].getValueBestCells()};
                for (int iRound{0}; iRound < numberOfRounds; ++iRound)
                {
                    V1[iRound][iGame] += valueBestCells[0][iRound];
                    V2[iRound][iGame] += valueBestCells[1][iRound];
                    V3[iRound][iGame] += valueBestCells[2][iRound];
                }
                // Update the scores
                const double normalizedScore{game.getScoreOfPlayer(iPlayer) / static_cast<double>(Smax)};
                S[iPlayer + numberOfPlayers * iGame] = normalizedScore;
                S_team[iGame] += normalizedScore;
            }
            for (int iRound{0}; iRound < numberOfRounds; ++iRound)
            {
                B1[iRound][iGame] /= numberOfPlayers;
                B2[iRound][iGame] /= numberOfPlayers;
                B3[iRound][iGame] /= numberOfPlayers;
                V1[iRound][iGame] /= numberOfPlayers;
                V2[iRound][iGame] /= numberOfPlayers;
                V3[iRound][iGame] /= numberOfPlayers;
                VB1[iRound][iGame] /= numberOfPlayers;
                VB2[iRound][iGame] /= numberOfPlayers;
                VB3[iRound][iGame] /= numberOfPlayers;
            }
            S_team[iGame] /= numberOfPlayers;

            // Ranking
            std::vector<std::size_t> idx{sortIndexes(std::vector<double>(S.begin() + numberOfPlayers * iGame,
                                                                         S.begin() + numberOfPlayers * iGame + numberOfPlayers))};
            for (int iRank{0}; iRank < numberOfPlayers; ++iRank)
            {
                switch (players[idx[iRank]].getPlayerType())
                {
                case PlayerType::collaborator:
                    ++rankCollaborator[iRank];
                    break;
                case PlayerType::neutral:
                    ++rankNeutral[iRank];
                    break;
                case PlayerType::defector:
                    ++rankDefector[iRank];
                    // ++N_def[iGame];
                    break;
                default:
                    break;
                }
            }
        }

        std::vector<double> propCollaborator(numberOfPlayers, 0);
        std::vector<double> propNeutral(numberOfPlayers, 0);
        std::vector<double> propDefector(numberOfPlayers, 0);
        for (int iRank{0}; iRank < numberOfPlayers; ++iRank)
        {
            propCollaborator[iRank] = static_cast<double>(rankCollaborator[iRank]) / numberOfGames;
            propNeutral[iRank] = static_cast<double>(rankNeutral[iRank]) / numberOfGames;
            propDefector[iRank] = static_cast<double>(rankDefector[iRank]) / numberOfGames;
        }

        std::vector<double> probaFind99(numberOfRounds, 0.);
        std::vector<double> probaFind80(numberOfRounds, 0.);
        std::vector<double> probaFind70(numberOfRounds, 0.);
        for (int iRound{0}; iRound < numberOfRounds; ++iRound)
        {
            probaFind99[iRound] = static_cast<double>(find99[iRound]) / (numberOfPlayers * numberOfGames);
            probaFind80[iRound] = static_cast<double>(find80[iRound]) / (numberOfPlayers * numberOfGames * 4);
            probaFind70[iRound] = static_cast<double>(find70[iRound]) / (numberOfPlayers * numberOfGames * 4);
        }

        // Write the observables in different files
        const std::string ruleStr{rule == Rule::rule_1 ? "rule_1" : "rule_2"};
        const std::string pathObservables{pathDataFigures + "pred_20_players/" + ruleStr + "/"};
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
        saveObservable(pathObservables + "B1", getAverage2dVector(B1));
        saveObservable(pathObservables + "B2", getAverage2dVector(B2));
        saveObservable(pathObservables + "B3", getAverage2dVector(B3));
        saveObservable(pathObservables + "V1", getAverage2dVector(V1));
        saveObservable(pathObservables + "V2", getAverage2dVector(V2));
        saveObservable(pathObservables + "V3", getAverage2dVector(V3));
        saveObservable(pathObservables + "VB1", getAverage2dVector(VB1));
        saveObservable(pathObservables + "VB2", getAverage2dVector(VB2));
        saveObservable(pathObservables + "VB3", getAverage2dVector(VB3));
        saveObservable(pathObservables + "S", getHistogram(S, 50));
        saveObservable(pathObservables + "S_team", getHistogram(S_team, 50));
        saveObservable(pathObservables + "<S>", std::vector<double>{getAverage(S)});
        saveObservable(pathObservables + "rk_col", propCollaborator);
        saveObservable(pathObservables + "rk_neu", propNeutral);
        saveObservable(pathObservables + "rk_def", propDefector);
        saveObservable(pathObservables + "proba_find_99", probaFind99);
        saveObservable(pathObservables + "proba_find_86_85_84", probaFind80);
        saveObservable(pathObservables + "proba_find_72_71", probaFind70);
    }
    return 0;
}
