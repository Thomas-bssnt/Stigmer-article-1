#ifndef RANDOM_H
#define RANDOM_H

#include <random>

namespace myRandom
{
    thread_local static std::random_device randomDevice;
    thread_local static std::mt19937 engine(randomDevice());
}

double randomNumber();

double randomNumber(double min, double max);

int randomNumber(int min, int max);

std::size_t randomIndexWithWeight(const std::vector<double> &weights);

template <typename T, typename U>
T randomChoiceWithWeight(const std::vector<T> &population, const std::vector<U> &weights);

#include "random.tpp"

#endif
