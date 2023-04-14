#ifndef RULE_H
#define RULE_H

#include <string>
#include <vector>

// Rule

class Rule
{
public:
    const std::string &getName() const;

    int getMaxRatingPerRound() const;

    int getMinRating() const;

    int getMaxRating() const;

    virtual int calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const = 0;

    virtual ~Rule() = default;

    static std::unique_ptr<Rule> createRule(int ruleNumber);

protected:
    const std::string m_name;
    const int m_minRating;
    const int m_maxRating;
    const int m_maxRatingPerRound;

    Rule(const std::string &name, int minRating, int maxRating, int maxNumberStars);
};

// Rule_1: Rule

class Rule_1 : public Rule
{
public:
    Rule_1();

    int calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const override;
};

// Rule_2: Rule

class Rule_2 : public Rule
{
public:
    Rule_2();

    int calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const override;
};

// Rule_3: Rule

class Rule_3 : public Rule
{
public:
    Rule_3();

    int calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const override;
};

// Rule_4: Rule

class Rule_4 : public Rule
{
public:
    Rule_4();

    int calculateScore(const std::vector<int> &values, const std::vector<int> &stars) const override;

private:
    const int m_scoreRemainingStars;
};

#endif
