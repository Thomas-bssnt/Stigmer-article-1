#include <algorithm> // std::copy, std::shuffle
#include <stdexcept> // std::invalid_argument
#include <vector>

#include "Map.h"
#include "random.h"

Map::Map(int numberOfCells, bool random)
    : m_numberOfCells{numberOfCells},
      m_values{generateValues(numberOfCells, random)}
{
}

int Map::getNumberOfCells() const
{
    return m_numberOfCells;
}

const std::vector<int> &Map::getValues() const
{
    return m_values;
}

std::vector<int> Map::generateValues(int numberOfCells, bool random)
{
    const std::vector<int> baseValues{99, 86, 86, 85, 84, 72, 72, 71, 71, 53, 53, 53, 51, 46, 45,
                                      45, 44, 44, 44, 43, 43, 28, 27, 27, 27, 24, 24, 24, 22, 21,
                                      20, 20, 20, 20, 20, 19, 19, 14, 14, 13, 13, 13, 12, 12, 11,
                                      9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6,
                                      6, 6, 6, 6, 6, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
                                      4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                      3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                      2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
                                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                      1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (numberOfCells == 0 || numberOfCells % baseValues.size() != 0)
    {
        throw std::invalid_argument("The number of cells must be a multiple of 225.");
    }

    std::vector<int> values(numberOfCells, 0);
    for (int i = 0; i < numberOfCells / baseValues.size(); ++i) // Copy the base values numberOfCells / 225 times.
    {
        std::copy(baseValues.begin(), baseValues.end(), values.begin() + i * baseValues.size());
    }

    if (random)
    {
        std::shuffle(values.begin(), values.end(), myRandom::engine);
    }

    return values;
}
