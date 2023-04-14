#ifndef GAME_EXCEPTION_H
#define GAME_EXCEPTION_H

#include <exception>
#include <string>

class GameException : public std::exception
{
private:
    std::string m_error;

public:
    GameException(std::string error) : m_error{error} {}

    const char *what() const noexcept override { return m_error.c_str(); }
};

#endif
