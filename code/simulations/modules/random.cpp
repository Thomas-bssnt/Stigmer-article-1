#include <iostream>
#include <random>

#include "random.h"

/*
 * Produces random double values uniformly distributed in the interval [0, 1).
 */
double randomNumber()
{
    return std::uniform_real_distribution(0., 1.)(myRandom::engine);
}

/*
 * Produces random double values uniformly distributed in the closed interval [min, max].
 */
double randomNumber(double min, double max)
{
    return std::uniform_real_distribution(min, max)(myRandom::engine);
}

/*
 * Produces random integer values uniformly distributed in the closed interval [min, max].
 */
int randomNumber(int min, int max)
{
    return std::uniform_int_distribution(min, max)(myRandom::engine);
}

/*
 * Select a random index from the weights vector according to the given weights.
 */
std::size_t randomIndexWithWeight(const std::vector<double> &weights)
{
    return std::discrete_distribution<std::size_t>(weights.begin(), weights.end())(myRandom::engine);
}
