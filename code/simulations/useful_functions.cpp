#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "json.hpp"

#include "Agent.h"
#include "AgentOpeningStrategy.h"
#include "Game.h"
#include "Rule.h"
#include "random.h"
#include "useful_functions.h"

std::vector<Agent> initializeAgents(const nlohmann::json &parametersAgentsType, const std::string &initializationType,
                                    Game &game, const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars)
{
    if (initializationType == "numbers")
    {
        return initializeAgentsFromNumbers(parametersAgentsType["numbers"], game, parametersVisits, parametersStars);
    }
    else if (initializationType == "fractions")
    {
        return initializeAgentsFromFractions(parametersAgentsType["number_of_players"], parametersAgentsType["fractions"],
                                             game, parametersVisits, parametersStars);
    }
    else
    {
        return std::vector<Agent>();
    }
}

std::vector<Agent> initializeAgents(const nlohmann::json &parametersAgentsType, const int &numberOfAgents,
                                    Game &game, const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars)
{
    return initializeAgentsFromFractions(numberOfAgents, parametersAgentsType["fractions"],
                                         game, parametersVisits, parametersStars);
}

std::vector<Agent> initializeAgentsFromNumbers(const std::vector<std::vector<int>> &numbers, Game &game,
                                               const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars)
{
    return initializeAgents(numbers[randomNumber(0, numbers.size() - 1)], game, parametersVisits, parametersStars);
}

std::vector<Agent> initializeAgentsFromFractions(int numberOfAgents, const std::vector<double> &fractions, Game &game,
                                                 const std::vector<double> &parametersVisits,
                                                 const nlohmann::json &parametersStars)
{
    std::vector<int> numbers(fractions.size(), 0);
    for (int iAgent{0}; iAgent < numberOfAgents; ++iAgent)
    {
        ++numbers[getAgentTypeNumbers(fractions)];
    }

    const std::vector<Agent> players = initializeAgents(numbers, game, parametersVisits, parametersStars);
    // std::cerr << "1: ";
    // std::cerr << players[0].m_ratingStrategy.m_parametersVisits.size() << " ";
    // for (const auto &parameter : players[0].m_ratingStrategy.m_parametersVisits)
    // {
    //     std::cerr << parameter << " ";
    // }
    // std::cerr << std::endl;
    return players;

    return initializeAgents(numbers, game, parametersVisits, parametersStars);
}

std::vector<Agent> initializeAgents(const std::vector<int> &numbers, Game &game,
                                    const std::vector<double> &parametersVisits, const nlohmann::json &parametersStars)
{
    const AgentOpeningStrategy openingStrategy{parametersVisits};
    std::vector<Agent> players;
    for (int iAgent{0}; iAgent < numbers[0]; ++iAgent)
    {
        // std::cerr << "Agent " << iAgent << " def" << std::endl;
        players.emplace_back(game.getAddress(), openingStrategy, parametersStars["def"], AgentType::defector);
    }
    for (int iAgent{0}; iAgent < numbers[1]; ++iAgent)
    {
        // std::cerr << "Agent " << iAgent << " neu" << std::endl;
        players.emplace_back(game.getAddress(), openingStrategy, parametersStars["neu"], AgentType::neutral);
    }
    for (int iAgent{0}; iAgent < numbers[2]; ++iAgent)
    {
        // std::cerr << "Agent " << iAgent << " col" << std::endl;
        players.emplace_back(game.getAddress(), openingStrategy, parametersStars["col"], AgentType::collaborator);
    }

    // for (const auto &parameter : players[0].m_ratingStrategy.m_parametersVisits)
    // {
    //     std::cerr << parameter << " ";
    // }
    // std::cerr << std::endl;

    return players;
}

int getAgentTypeNumbers(const std::vector<double> &fractions)
{
    const double p{randomNumber()};
    if (p <= fractions[0])
    {
        return 0;
    }
    else if (p <= fractions[0] + fractions[1])
    {
        return 1;
    }
    return 2;
}

int getScoreMax(const Rule &rule, const int numberOfRounds, const int Vmax1, const int Vmax2, const int Vmax3)
{
    if (rule.getName() == "rule_1" || rule.getName() == "rule_2")
    {
        return numberOfRounds * (Vmax1 + Vmax2 + Vmax3);
    }
    else if (rule.getName() == "rule_3" || rule.getName() == "rule_4")
    {
        return numberOfRounds * (Vmax1 * 5 + Vmax2 * 3);
    }
    else
    {
        return -1;
    }
}

std::vector<double> getParameters(const std::string &filePath)
{
    std::vector<double> bestParameters;
    std::ifstream outFile(filePath);
    if (outFile.is_open())
    {
        int iLine{-1};
        std::string lastLine;
        std::string nextLine;
        do
        {
            ++iLine;
            lastLine = nextLine;
        } while (std::getline(outFile, nextLine));

        if (iLine != 0) // if the file is not empty
        {
            int iNumber{0};
            std::string number;
            for (const auto x : lastLine + " ")
            {
                if (x == ' ')
                {
                    bestParameters.push_back(std::stod(number));
                    number = "";
                    ++iNumber;
                }
                else
                {
                    number += x;
                }
            }
        }
        else
        {
            std::cerr << "The file " << filePath << " is empty.\n";
        }
    }
    else
    {
        std::cerr << "The file " << filePath << " does not exist or could not be opened.\n";
    }
    return bestParameters;
}

double computeP(const std::vector<double> &starsDistribution, const std::vector<int> &values, const int Vmax)
{
    double P{0.};
    for (int iCell{0}; iCell < starsDistribution.size(); ++iCell)
    {
        P += starsDistribution[iCell] * values[iCell];
    }
    return P / Vmax;
}

double computeQ(const std::vector<double> &visitsDistribution, const std::vector<int> &values, const int Vmax1,
                const int Vmax2, const int Vmax3)
{
    double Q{0.};
    for (int iCell{0}; iCell < visitsDistribution.size(); ++iCell)
    {
        Q += visitsDistribution[iCell] * values[iCell];
    }
    return Q * 3. / (Vmax1 + Vmax2 + Vmax3);
}

double computeIPR(const std::vector<double> &distribution)
{
    double sumSquared{0.};
    for (const auto &value : distribution)
    {
        sumSquared += value * value;
    }
    return sumSquared == 0. ? 0. : 1. / sumSquared;
}

double computeF(const std::vector<double> &distribution, const std::vector<int> &values)
{
    double sumSqrt{0.};
    int sumValues{0};
    for (int iCell{0}; iCell < distribution.size(); ++iCell)
    {
        sumValues += values[iCell];
        sumSqrt += std::sqrt(distribution[iCell] * values[iCell]);
    }
    return sumSqrt / std::sqrt(sumValues);
}

double getMedian(const std::vector<double> &vector)
{
    std::vector<double> sortedVector{vector};
    std::sort(sortedVector.begin(), sortedVector.end());
    if (sortedVector.size() % 2)
    {
        return sortedVector[sortedVector.size() / 2];
    }
    return (sortedVector[sortedVector.size() / 2] + sortedVector[sortedVector.size() / 2 - 1]) / 2;
}

double getAverage(const std::vector<double> &vector)
{
    return std::accumulate(vector.begin(), vector.end(), 0.) / vector.size();
}

double getAverage(const std::vector<int> &vector)
{
    return std::accumulate(vector.begin(), vector.end(), 0.) / static_cast<int>(vector.size());
}

std::vector<double> getAverage2dVector(const std::vector<std::vector<double>> &vector2d)
{
    std::vector<double> averagedVector(vector2d.size(), 0.);
    for (int i{0}; i < vector2d.size(); ++i)
    {
        averagedVector[i] = getAverage(vector2d[i]);
    }
    return averagedVector;
}

std::vector<double> getHistogram(const std::vector<double> &score, const int numberOfDivisions)
{
    // Do the histogram
    std::vector<int> counts(numberOfDivisions, 0);
    for (auto &value : score)
    {
        int binNumber{static_cast<int>(value < 1 ? value * numberOfDivisions : numberOfDivisions - 1)};
        counts[binNumber]++;
    }

    // Normalization (norm 1)
    std::vector<double> countsNormalized(numberOfDivisions, 0.);
    for (int iDivisions{0}; iDivisions < numberOfDivisions; ++iDivisions)
    {
        countsNormalized[iDivisions] = static_cast<double>(counts[iDivisions]) / score.size() * numberOfDivisions;
    }
    return countsNormalized;
}

std::vector<double> getCounts(std::vector<int> &ranks)
{
    std::vector<double> counts(5, 0.);
    for (auto &rank : ranks)
    {
        ++counts[rank - 1];
    }
    for (auto &count : counts)
    {
        count /= static_cast<double>(ranks.size());
    }
    return counts;
}

std::vector<double> getL1Norm(std::vector<int> &vector)
{
    int sum{0};
    for (auto &value : vector)
    {
        sum += value;
    }
    std::vector<double> normalizedVector(vector.size(), 0);
    for (int i{0}; i < vector.size(); ++i)
    {
        normalizedVector[i] = vector[i] / static_cast<double>(sum);
    }
    return normalizedVector;
}

void getS_team_N_def(const std::vector<double> &teamScore, const std::vector<int> &numberDefectorGame,
                     std::vector<double> &mean, std::vector<double> &median)
{
    std::vector<int> numberGamesWithNdefectors(mean.size(), 0);
    for (int iGame{0}; iGame < teamScore.size(); ++iGame)
    {
        ++numberGamesWithNdefectors[numberDefectorGame[iGame]];
    }

    for (int iNumberDefector{0}; iNumberDefector < numberGamesWithNdefectors.size(); ++iNumberDefector)
    {
        std::vector<double> scores(numberGamesWithNdefectors[iNumberDefector], 0);
        int i{0};
        for (int iGame{0}; iGame < teamScore.size(); ++iGame)
        {
            if (numberDefectorGame[iGame] == iNumberDefector)
            {
                scores[i] = teamScore[iGame];
                ++i;
            }
        }
        if (numberGamesWithNdefectors[iNumberDefector] != 0)
        {
            mean[iNumberDefector] = getAverage(scores);
            median[iNumberDefector] = getMedian(scores);
        }
    }
}

std::vector<std::size_t> sortIndexes(const std::vector<double> &vector)
{
    std::vector<std::size_t> idx(vector.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::stable_sort(idx.begin(), idx.end(),
                     [&vector](std::size_t i1, std::size_t i2)
                     { return vector[i1] > vector[i2]; });
    return idx;
}

void saveObservable(const std::string &observablePath, const std::vector<double> &observable)
{
    std::ofstream file(observablePath + ".txt");
    if (file.is_open())
    {
        for (const auto &value : observable)
        {
            file << value << "\n";
        }
    }
    else
    {
        std::cerr << "The file " + observablePath + " could not be opened.\n";
    }
}
