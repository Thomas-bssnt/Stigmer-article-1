#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>

#include "Map.h"
#include "Rule.h"

class Game
{
public:
    Game(int numberOfRounds, int numberOfTurns, int numberOfPlayers, const Rule &rule, const Map &map,
         bool evaporation, double tauEvaporation);

    Game(int numberOfRounds, int numberOfPlayers, const Rule &rule, double tauEvaporation);

    Game(int numberOfRounds, int numberOfPlayers, const Rule &rule);

    int openCell(int playerId, int iCell);

    void rateCell(int playerId, int numberOfRatings);

    Game *getAddress();

    int getPlayerId();

    const std::vector<int> &getValues() const;

    const std::vector<double> &getColors() const;

    const std::vector<double> &getOpeningsDistribution() const;

    const std::vector<double> &getInstantaneousOpeningsDistribution() const;

    const std::vector<double> &getRatingsDistribution() const;

    const std::vector<double> &getInstantaneousRatingsDistribution() const;

    int getScoreOfPlayer(int playerId) const;

    int getNumberOfRounds() const;

    int getNumberOfTurns() const;

    int getNumberOfCells() const;

    int getCurrentRound() const;

    void save(const std::string &path, const std::string &filename) const;

private:
    void changeRound();

    void updateDistributions();

    void updateColors();

    void updateScores();

    bool hasThePlayerOpenedTheCellDuringTheRound(int playerId, int iCell) const;

    bool haveAllPlayersPlayedTheirTurns() const;

    template <typename T>
    static std::vector<double> normalize(const std::vector<T> &vector);

    // Game variables
    const int m_numberOfRounds;
    const int m_numberOfTurns;
    const int m_numberOfPlayers;
    // Rule
    const Rule &m_rule;
    const int m_minRating;
    const int m_maxRating;
    const int m_maxRatingPerRound;
    // Map
    const Map &m_map;
    const int m_numberOfCells;
    const std::vector<int> m_vMap;
    // Evaporation
    const bool m_evaporation;
    const double m_tauEvaporation;
    //
    std::vector<int> m_oMap;
    std::vector<int> m_rMap;
    std::vector<double> m_rColorsMap;
    // Distributions
    std::vector<double> m_colorsDistribution;
    std::vector<double> m_instantaneousOpeningsDistribution;
    std::vector<double> m_instantaneousRatingsDistribution;
    std::vector<double> m_openingsDistribution;
    std::vector<double> m_ratingsDistribution;
    // Scores
    std::vector<int> m_scores;
    // Game variables
    int m_playerCount;
    int m_iRound;
    // Variables reinitialized at each round
    std::vector<int> m_iTurn;
    std::vector<bool> m_hasOpenedACell;
    std::vector<int> m_numberOfRatingsRemaining;
    std::vector<std::vector<std::vector<int>>> m_iCellOpened;
    std::vector<std::vector<std::vector<int>>> m_oCellOpened;
    std::vector<std::vector<std::vector<int>>> m_rCellOpened;
};

#endif
