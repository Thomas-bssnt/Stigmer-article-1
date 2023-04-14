#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

#include "Game.h"
#include "GameException.hpp"
#include "Map.h"
#include "Rule.h"

Game::Game(int numberOfRounds, int numberOfTurns, int numberOfPlayers, const Rule &rule, const Map &map,
           bool evaporation, double tauEvaporation)
    : m_numberOfRounds{numberOfRounds},
      m_numberOfTurns{numberOfTurns},
      m_numberOfPlayers{numberOfPlayers},
      m_rule{rule},
      m_minRating{rule.getMinRating()},
      m_maxRating{rule.getMaxRating()},
      m_maxRatingPerRound{rule.getMaxRatingPerRound()},
      m_map{map},
      m_numberOfCells{map.getNumberOfCells()},
      m_vMap{map.getValues()},
      m_evaporation{evaporation},
      m_tauEvaporation{tauEvaporation},
      m_oMap{std::vector<int>(m_numberOfCells, 0)},
      m_rMap{std::vector<int>(m_numberOfCells, 0)},
      m_rColorsMap{std::vector<double>(m_numberOfCells, 0.)},
      m_colorsDistribution{std::vector<double>(m_numberOfCells, 0.)},
      m_instantaneousOpeningsDistribution{std::vector<double>(m_numberOfCells, 0.)},
      m_instantaneousRatingsDistribution{std::vector<double>(m_numberOfCells, 0.)},
      m_openingsDistribution{std::vector<double>(m_numberOfCells, 0.)},
      m_ratingsDistribution{std::vector<double>(m_numberOfCells, 0.)},
      m_scores{std::vector<int>(m_numberOfPlayers, 0)},
      m_playerCount{0},
      m_iRound{0},
      m_iTurn{std::vector<int>(numberOfPlayers, 0)},
      m_hasOpenedACell{std::vector<bool>(numberOfPlayers, false)},
      m_numberOfRatingsRemaining{std::vector<int>(numberOfPlayers, m_maxRatingPerRound)},
      m_iCellOpened{std::vector<std::vector<std::vector<int>>>(
          m_numberOfPlayers, std::vector<std::vector<int>>(m_numberOfRounds, std::vector<int>(numberOfTurns, 0)))},
      m_oCellOpened{std::vector<std::vector<std::vector<int>>>(
          m_numberOfPlayers, std::vector<std::vector<int>>(m_numberOfRounds, std::vector<int>(numberOfTurns, 0)))},
      m_rCellOpened{std::vector<std::vector<std::vector<int>>>(
          m_numberOfPlayers, std::vector<std::vector<int>>(m_numberOfRounds, std::vector<int>(numberOfTurns, 0)))}
{
}

Game::Game(int numberOfRounds, int numberOfPlayers, const Rule &rule, double tauEvaporation)
    : Game(numberOfRounds, 3, numberOfPlayers, rule, Map(225, false), true, tauEvaporation)
{
}

Game::Game(int numberOfRounds, int numberOfPlayers, const Rule &rule)
    : Game(numberOfRounds, 3, numberOfPlayers, rule, Map(225, false), false, 0.)
{
}

/*
 * Open a cell.
 * @param iPlayer the player number
 * @param iCell the cell number
 * @return the value of the cell
 */
int Game::openCell(int iPlayer, int iCell)
{
    if (m_iRound >= m_numberOfRounds)
    {
        throw GameException("Game::openCell: The game is over.");
    }
    if (m_iTurn[iPlayer] >= m_numberOfTurns)
    {
        throw GameException("Game::openCell: Player " + std::to_string(iPlayer) + " already played " +
                            std::to_string(m_numberOfTurns) + " times during the round.");
    }
    if (m_hasOpenedACell[iPlayer])
    {
        throw GameException("Game::openCell: Player " + std::to_string(iPlayer) +
                            " already opened a cell and must rate it before opening another cell.");
    }
    if (iCell < 0 || iCell >= m_numberOfCells)
    {
        throw GameException("Game::openCell: The cell number " + std::to_string(iCell) + " does not exist.");
    }
    if (hasThePlayerOpenedTheCellDuringTheRound(iPlayer, iCell))
    {
        throw GameException("Game::openCell: The player " + std::to_string(iPlayer) + " already opened the cell " +
                            std::to_string(iCell) + " during the round.");
    }

    m_hasOpenedACell[iPlayer] = true;
    m_iCellOpened[iPlayer][m_iRound][m_iTurn[iPlayer]] = iCell;
    m_oCellOpened[iPlayer][m_iRound][m_iTurn[iPlayer]] = m_vMap[iCell];
    return m_vMap[iCell];
}

/*
 * Rate a cell.
 * @param iPlayer the player number
 * @param rating the number of stars
 */
void Game::rateCell(int iPlayer, int rating)
{
    if (m_iRound >= m_numberOfRounds)
    {
        throw GameException("Game::rateCell: The game is over.");
    }
    if (m_iTurn[iPlayer] >= m_numberOfTurns)
    {
        throw GameException("Game::rateCell: Player " + std::to_string(iPlayer) + " already played " +
                            std::to_string(m_numberOfTurns) + " times during the round.");
    }
    if (!m_hasOpenedACell[iPlayer])
    {
        throw GameException("Game::rateCell: Player " + std::to_string(iPlayer) +
                            " must open a cell before rating it.");
    }
    if (rating < m_minRating || rating > m_maxRating)
    {
        throw GameException("Game::rateCell: Player " + std::to_string(iPlayer) +
                            " entered an invalid rating (" + std::to_string(rating) + " not in [" +
                            std::to_string(m_minRating) + ", " + std::to_string(m_maxRating) + "]).");
    }
    if (rating > m_numberOfRatingsRemaining[iPlayer])
    {
        throw GameException("Game::rateCell: Player " + std::to_string(iPlayer) + " only has " +
                            std::to_string(m_numberOfRatingsRemaining[iPlayer]) + " ratings remaining.");
    }

    m_rCellOpened[iPlayer][m_iRound][m_iTurn[iPlayer]] = rating;
    m_numberOfRatingsRemaining[iPlayer] -= rating;
    ++m_iTurn[iPlayer];
    m_hasOpenedACell[iPlayer] = false;

    if (haveAllPlayersPlayedTheirTurns())
    {
        changeRound();
    }
}

/*
 * Change the round and update the distributions and scores.
 */
void Game::changeRound()
{
    updateDistributions();
    updateColors();
    updateScores();

    m_iTurn = std::vector<int>(m_numberOfPlayers, 0);
    m_numberOfRatingsRemaining = std::vector<int>(m_numberOfPlayers, m_maxRatingPerRound);

    ++m_iRound;
}

/*
 * Update the distributions.
 */
void Game::updateDistributions()
{
    std::vector<int> oMapRound(m_numberOfCells, 0);
    std::vector<int> rMapRound(m_numberOfCells, 0);
    for (int iPlayer{0}; iPlayer < m_numberOfPlayers; ++iPlayer)
    {
        for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
        {
            const int iCell{m_iCellOpened[iPlayer][m_iRound][iTurn]};
            const int rating{m_rCellOpened[iPlayer][m_iRound][iTurn]};

            ++oMapRound[iCell];
            rMapRound[iCell] += rating;

            ++m_oMap[iCell];
            m_rMap[iCell] += rating;
        }
    }
    m_instantaneousOpeningsDistribution = normalize(oMapRound);
    m_instantaneousRatingsDistribution = normalize(rMapRound);
    m_openingsDistribution = normalize(m_oMap);
    m_ratingsDistribution = normalize(m_rMap);
}

/*
 * Update the color map used by players.
 */
void Game::updateColors()
{
    if (!m_evaporation)
    {
        m_colorsDistribution = m_ratingsDistribution;
    }
    else
    {
        const double evaporationFactor{1. - 1. / m_tauEvaporation};
        for (int iCell{0}; iCell < m_numberOfCells; ++iCell)
        {
            m_rColorsMap[iCell] *= evaporationFactor;
        }

        for (int iPlayer{0}; iPlayer < m_numberOfPlayers; ++iPlayer)
        {
            for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
            {
                const int iCell{m_iCellOpened[iPlayer][m_iRound][iTurn]};
                const int rating{m_rCellOpened[iPlayer][m_iRound][iTurn]};
                m_rColorsMap[iCell] += rating;
            }
        }
        m_colorsDistribution = normalize(m_rColorsMap);
    }
}

/*
 * Update the scores of the players.
 */
void Game::updateScores()
{
    for (int iPlayer{0}; iPlayer < m_numberOfPlayers; ++iPlayer)
    {
        m_scores[iPlayer] += m_rule.calculateScore(m_oCellOpened[iPlayer][m_iRound], m_rCellOpened[iPlayer][m_iRound]);
    }
}

/*
 * Check if a player has already opened a cell during the round.
 *
 * @param iPlayer the player number
 * @param iCell the cell number
 * @return true if the player has already opened the cell during the round.
 */
bool Game::hasThePlayerOpenedTheCellDuringTheRound(int iPlayer, int iCell) const
{
    return std::any_of(m_iCellOpened[iPlayer][m_iRound].begin(),
                       m_iCellOpened[iPlayer][m_iRound].begin() + m_iTurn[iPlayer],
                       [iCell](int iCellOpened)
                       { return iCellOpened == iCell; });
}

/*
 * Check if all players have played their turns.
 *
 * @return true if all players have played their turns.
 */
bool Game::haveAllPlayersPlayedTheirTurns() const
{
    return std::all_of(m_iTurn.begin(),
                       m_iTurn.end(),
                       [&numberOfTurns = m_numberOfTurns](int iTurn)
                       { return iTurn == numberOfTurns; });
}

/*
 * Save the game data into a CSV file.
 *
 * @param path the path to the folder where the CSV file will be saved.
 * @param filename the name of the CSV file.
 */
void Game::save(const std::string &path, const std::string &filename) const
{
    // TODO: only work for Rule 2

    std::ofstream file(path + filename + ".csv");
    if (file.is_open())
    {
        file << "round,playerId,mapX,mapY,value,numberStars,score\n";
        for (int iRound{0}; iRound < m_numberOfRounds; ++iRound)
        {
            for (int iPlayer{0}; iPlayer < m_numberOfPlayers; ++iPlayer)
            {
                for (int iTurn{0}; iTurn < m_numberOfTurns; ++iTurn)
                {
                    file << iRound + 1 << ","
                         << iPlayer + 1 << ","
                         << m_iCellOpened[iPlayer][iRound][iTurn] % 15 << ","
                         << m_iCellOpened[iPlayer][iRound][iTurn] / 15 << ","
                         << m_oCellOpened[iPlayer][iRound][iTurn] << ","
                         << m_rCellOpened[iPlayer][iRound][iTurn] << ","
                         << m_oCellOpened[iPlayer][iRound][iTurn] << "\n";
                }
            }
        }
    }
    else
    {
        std::cerr << "The file " + path + " could not be opened.\n";
    }
}

Game *Game::getAddress()
{
    return this;
}

/*
 * Assigns a unique playerId to a player. This method should only be used once per player.
 *
 * @return the assigned playerId.
 */
int Game::getPlayerId()
{
    return m_playerCount++;
}

const std::vector<int> &Game::getValues() const
{
    return m_vMap;
}

const std::vector<double> &Game::getColors() const
{
    return m_colorsDistribution;
}

const std::vector<double> &Game::getOpeningsDistribution() const
{
    return m_openingsDistribution;
}

const std::vector<double> &Game::getInstantaneousOpeningsDistribution() const
{
    return m_instantaneousOpeningsDistribution;
}

const std::vector<double> &Game::getRatingsDistribution() const
{
    return m_ratingsDistribution;
}

const std::vector<double> &Game::getInstantaneousRatingsDistribution() const
{
    return m_instantaneousRatingsDistribution;
}

int Game::getScoreOfPlayer(int iPlayer) const
{
    return m_scores[iPlayer];
}

int Game::getNumberOfRounds() const
{
    return m_numberOfRounds;
}

int Game::getNumberOfTurns() const
{
    return m_numberOfTurns;
}

int Game::getNumberOfCells() const
{
    return m_numberOfCells;
}

int Game::getCurrentRound() const
{
    return m_iRound;
}

/*
 * Calculate the L1 norm of a vector.
 *
 * @param vector the vector to normalize. All elements of the vector must be non-negative.
 * @return the L1 norm of the input vector. If the sum of the values in the vector is 0., then a vector full of 0.
 * is returned.
 */
template <typename T>
std::vector<double> Game::normalize(const std::vector<T> &vector)
{
    const double sum{std::accumulate(vector.begin(), vector.end(), 0.)};
    std::vector<double> normalizedVector(vector.size(), 0.);
    if (sum != 0.)
    {
        for (int i{0}; i < vector.size(); ++i)
        {
            normalizedVector[i] = vector[i] / sum;
        }
    }
    return normalizedVector;
}
