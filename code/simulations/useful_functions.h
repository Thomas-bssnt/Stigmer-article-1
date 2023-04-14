#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>
#include <vector>

#include "json.hpp"

#include "Agent.h"
#include "Game.h"

std::vector<Agent> initializeAgents(const nlohmann::json &parametersAgentsType, const std::string &initializationType,
                                    Game &game, const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars);

std::vector<Agent> initializeAgents(const nlohmann::json &parametersAgentsType, const int &numberOfAgents,
                                    Game &game, const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars);

std::vector<Agent> initializeAgentsFromNumbers(const std::vector<std::vector<int>> &numbers, Game &game,
                                               const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars);

std::vector<Agent> initializeAgentsFromFractions(int numberOfAgents, const std::vector<double> &fractions, Game &game,
                                                 const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars);

std::vector<Agent> initializeAgents(const std::vector<int> &numbers, Game &game,
                                    const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars);

int getAgentTypeNumbers(const std::vector<double> &fractions);

int getScoreMax(const Rule &rule, int numberOfRounds, int Vmax1, int Vmax2, int Vmax3);

std::vector<double> getParameters(const std::string &filePath);

double computeP(const std::vector<double> &starsDistribution, const std::vector<int> &values, int Vmax);

double computeQ(const std::vector<double> &visitsDistribution, const std::vector<int> &values,
                int Vmax1, int Vmax2, int Vmax3);

double computeIPR(const std::vector<double> &distribution);

double computeF(const std::vector<double> &distribution, const std::vector<int> &values);

double getMedian(const std::vector<double> &vector);

double getAverage(const std::vector<double> &vector);

double getAverage(const std::vector<int> &vector);

std::vector<double> getAverage2dVector(const std::vector<std::vector<double>> &vector2d);

std::vector<double> getL1Norm(std::vector<int> &vector);

std::vector<double> getHistogram(const std::vector<double> &score, int numberOfDivisions);

std::vector<double> getCounts(std::vector<int> &ranks);

void getS_team_N_def(const std::vector<double> &teamScore, const std::vector<int> &numberDefectorGame,
                     std::vector<double> &mean, std::vector<double> &median);

std::vector<std::size_t> sortIndexes(const std::vector<double> &vector);

void saveObservable(const std::string &observablePath, const std::vector<double> &observable);

#endif
