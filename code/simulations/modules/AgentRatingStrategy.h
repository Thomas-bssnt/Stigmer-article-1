#ifndef AGENT_RATING_STRATEGY_H
#define AGENT_RATING_STRATEGY_H

#include <vector>

#include "json.hpp"

#include "Cell.h"

class AgentRatingStrategy
{
public:
    AgentRatingStrategy(int minRating, int maxRating, const nlohmann::json &parameters);

    AgentRatingStrategy(const nlohmann::json &parameters);

    int choseRating(int value) const;

private:
    const nlohmann::json m_parameters;
    std::vector<int> m_ratings;

    std::vector<double> computeProbabilities(int value) const;

    void computeProbabilitiesMNSLinear(std::vector<double> &probabilities,
                                       const std::vector<double> &parameters, int value) const;

    static double functionTanh(const std::vector<double> &parameters, int value);

    static double functionConstant(const std::vector<double> &parameters, int value);

    static double functionLinear(const std::vector<double> &parameters, int value);

    static double functionGaussian(const std::vector<double> &parameters, int value);

    static nlohmann::json to_json(const std::vector<double> &parametersStars, const std::string &functionType);
};

#endif
