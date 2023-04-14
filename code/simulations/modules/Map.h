#ifndef MAP_H
#define MAP_H

#include <vector>

class Map
{
public:
    Map(int numberOfCells, bool random);

    int getNumberOfCells() const;

    const std::vector<int> &getValues() const;

private:
    const int m_numberOfCells;
    const std::vector<int> m_values;

    static std::vector<int> generateValues(int numberOfCells, bool random);
};

#endif
