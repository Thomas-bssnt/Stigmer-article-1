#include <numeric>   // std::accumulate, std::inner_product
#include <stdexcept> // std::invalid_argument
#include <string>
#include <vector>

#include "Rule.h"

// Rule

Rule::Rule(const std::string &name, int minRating, int maxRating, int maxStarsPerRound)
    : m_name{name},
      m_minRating{minRating},
      m_maxRating{maxRating},
      m_maxRatingPerRound{maxStarsPerRound}
{
}

const std::string &Rule::getName() const { return m_name; }

int Rule::getMaxRatingPerRound() const { return m_maxRatingPerRound; }

int Rule::getMinRating() const { return m_minRating; }

int Rule::getMaxRating() const { return m_maxRating; }

std::unique_ptr<Rule> Rule::createRule(int ruleNumber)
{
    switch (ruleNumber)
    {
    case 1:
        return std::make_unique<Rule_1>();
    case 2:
        return std::make_unique<Rule_2>();
    case 3:
        return std::make_unique<Rule_3>();
    case 4:
        return std::make_unique<Rule_4>();
    default:
        throw std::invalid_argument{"Invalid rule number"};
    }
}

// Rule_1: Rule

Rule_1::Rule_1() : Rule{"rule_1", 0, 5, 15} {}

int Rule_1::calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const
{
    return 0;
}

// Rule_2: Rule

Rule_2::Rule_2() : Rule{"rule_2", 0, 5, 15} {}

int Rule_2::calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const
{
    return std::accumulate(values.begin(), values.end(), 0);
}

// Rule_3: Rule

Rule_3::Rule_3() : Rule{"rule_3", 0, 5, 8} {}

int Rule_3::calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const
{
    return std::inner_product(values.begin(), values.end(), stars.begin(), 0);
}

// Rule_4: Rule

Rule_4::Rule_4() : Rule{"rule_4", 0, 5, 8}, m_scoreRemainingStars{50} {}

int Rule_4::calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const
{
    return std::inner_product(values.begin(), values.end(), stars.begin(), 0) +
           std::accumulate(stars.begin(), stars.end(), m_maxRatingPerRound, std::minus<>()) * m_scoreRemainingStars;
}
