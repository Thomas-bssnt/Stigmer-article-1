#ifndef RANDOM_TPP
#define RANDOM_TPP

#ifndef RANDOM_H
#error __FILE__ should only be included from random.h.
#endif

#include <stdexcept> // std::invalid_argument

/*
 * Select a random element from the population vector according to the given weights.
 *
 * @param population: vector of elements to choose from.
 * @param weights: vector of weights associated to the elements of the population vector.
 * @return: a random element from the population vector according to the given weights.
 */
template <typename T, typename U>
T randomChoiceWithWeight(const std::vector<T> &population, const std::vector<U> &weights)
{
    if (population.size() != weights.size())
    {
        throw std::invalid_argument("Population and weights vectors must have the same size.");
    }

    return population[std::discrete_distribution<int>(weights.begin(), weights.end())(myRandom::engine)];
}

#endif
