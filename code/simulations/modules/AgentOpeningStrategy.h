#ifndef AGENT_OPENING_STRATEGY_H
#define AGENT_OPENING_STRATEGY_H

#include <vector>

#include "Cell.h"

class AgentOpeningStrategy
{
public:
    AgentOpeningStrategy(const std::vector<double> &parametersExploration,
                         const std::vector<std::vector<double>> &parametersReplayCell);

    AgentOpeningStrategy(const std::vector<double> &parametersOpenings);

    int choseCell(int round, const std::vector<double> &colors, const std::vector<Cell> &bestCells,
                  const std::vector<Cell> &cellsPlayed);

private:
    const std::vector<double> m_parametersExploration;
    const std::vector<std::vector<double>> m_parametersReplayCell;
    std::vector<double> m_exploringProbabilities;
    int m_round;

    void updateExploringProbabilities(const std::vector<double> &colors);

    bool shouldReplayCell(int value, int iTurn) const;

    static int chooseACellByExploring(int round, const std::vector<double> &exploringProbabilities,
                                      const std::vector<Cell> &bestCells, const std::vector<Cell> &cellsPlayed);

    static std::vector<std::vector<double>> to2d(const std::vector<double> &vector1d, const int nRows);
};

#endif
