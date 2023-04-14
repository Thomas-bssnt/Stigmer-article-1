#ifndef AGENT_H
#define AGENT_H

#include <vector>

#include "AgentOpeningStrategy.h"
#include "AgentRatingStrategy.h"
#include "Cell.h"
#include "Game.h"

enum class AgentType
{
    collaborator,
    neutral,
    defector,
    optimized,
};

class Agent
{
public:
    Agent(Game *pGame, const AgentOpeningStrategy &openingStrategy,
          const AgentRatingStrategy &ratingStrategy, const AgentType &AgentType);

    // Agent(Game *pGame, const std::vector<double> &parametersVisits, const std::vector<double> &parametersStars,
    //       const std::string &functionType, const AgentType &AgentType);

    void playARound();

    const std::vector<std::vector<int>> &getPlayBestCellsRound() const;

    const std::vector<std::vector<int>> &getValueBestCells() const;

    const AgentType &getAgentType() const;

    const std::vector<bool> &getFoundCells() const;

private:
    void updatePlayBestCells(const std::vector<Cell> &cellsPlayed);

    void updateBestCells(const std::vector<Cell> &cellsPlayed);

    // Game variables
    Game *mp_Game;
    const int m_iAgent;
    const int m_numberOfTurns;
    const int m_numberOfCells;
    // Strategy variables
    AgentOpeningStrategy m_openingStrategy;
    const AgentRatingStrategy m_ratingStrategy;
    //
    int m_round;
    // const std::vector<double> m_parametersVisits;
    // const nlohmann::json m_parametersStars;
    const AgentType m_AgentType;
    std::vector<Cell> m_bestCells;
    std::vector<std::vector<int>> m_playBestCells;
    std::vector<std::vector<int>> m_valueBestCells;
    std::vector<bool> m_foundCells;
};

#endif
