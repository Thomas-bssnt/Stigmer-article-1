#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "Agent.h"
#include "AgentOpeningStrategy.h"
#include "AgentRatingStrategy.h"
#include "Cell.h"
#include "random.h"

Agent::Agent(Game *pGame, const AgentOpeningStrategy &openingStrategy,
             const AgentRatingStrategy &ratingStrategy, const AgentType &AgentType)
    : mp_Game{pGame},
      m_iAgent{mp_Game->getPlayerId()},
      m_numberOfTurns{mp_Game->getNumberOfTurns()},
      m_numberOfCells{mp_Game->getNumberOfCells()},
      m_openingStrategy{openingStrategy},
      m_ratingStrategy{ratingStrategy},
      m_round{0},
      m_AgentType{AgentType},
      m_bestCells{std::vector<Cell>(m_numberOfTurns, {-1, -1})},
      m_playBestCells{std::vector<std::vector<int>>(m_numberOfTurns, std::vector<int>(mp_Game->getNumberOfRounds(), 0))},
      m_valueBestCells{std::vector<std::vector<int>>(m_numberOfTurns, std::vector<int>(mp_Game->getNumberOfRounds(), 0))},
      m_foundCells{std::vector<bool>(m_numberOfCells, false)}
{
    assert(m_numberOfTurns == 3);
}

// Agent::Agent(Game *pGame, const std::vector<double> &parametersVisits,
//              const std::vector<double> &parametersStars, const std::string &functionType,
//              const AgentType &AgentType)
//     : Agent(pGame, parametersVisits, to_json(parametersStars, functionType), AgentType)
// {
// }

// TODO: move m_foundCells in Game

void Agent::playARound()
{
    m_round = mp_Game->getCurrentRound();
    std::vector<Cell> cellsPlayed;
    for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
    {
        const int iCell{m_openingStrategy.choseCell(m_round, mp_Game->getColors(), m_bestCells, cellsPlayed)};
        const int vCell{mp_Game->openCell(m_iAgent, iCell)};
        mp_Game->rateCell(m_iAgent, m_ratingStrategy.choseRating(vCell));
        cellsPlayed.push_back({iCell, vCell});
        m_foundCells[iCell] = true;
    }
    updatePlayBestCells(cellsPlayed);
    updateBestCells(cellsPlayed);

    for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
    {
        m_valueBestCells[iTurn][m_round] = m_bestCells[iTurn].value;
    }
}

void Agent::updatePlayBestCells(const std::vector<Cell> &cellsPlayed)
{
    for (auto &cellPlayed : cellsPlayed)
    {
        for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
        {
            if (cellPlayed.index == m_bestCells[iTurn].index)
            {
                ++m_playBestCells[iTurn][m_round];
            }
        }
    }
}

void Agent::updateBestCells(const std::vector<Cell> &cellsPlayed)
{
    m_bestCells = std::vector<Cell>(m_numberOfTurns, {-1, -1});
    for (const auto &cellPlayed : cellsPlayed)
    {
        for (int i = 0; i < m_bestCells.size(); ++i)
        {
            if (cellPlayed.value > m_bestCells[i].value)
            {
                m_bestCells.insert(m_bestCells.begin() + i, cellPlayed);
                m_bestCells.pop_back();
                break;
            }
        }
    }
}

const std::vector<std::vector<int>> &Agent::getPlayBestCellsRound() const
{
    return m_playBestCells;
}

const std::vector<std::vector<int>> &Agent::getValueBestCells() const
{
    return m_valueBestCells;
}

const std::vector<bool> &Agent::getFoundCells() const
{
    return m_foundCells;
}

const AgentType &Agent::getAgentType() const
{
    return m_AgentType;
}