#include "Freddy.hpp"

Freddy::Freddy(void) : m_kills()
{
}

Freddy::Freddy(const Freddy &other) : m_kills(other.m_kills)
{
    *this = other;
}

Freddy &Freddy::operator=(const Freddy &other)
{
    if (this != &other)
    {
        *this = other;
    }
    return (*this);
}

Freddy::~Freddy()
{
}

void Freddy::sing()
{
    std::cout << "One, two, Freddy's coming for you\n"
              << "Three, four, better lock your door\n"
              << "Five, six, grab your crucifix\n"
              << "Seven, eight, gonna stay up late\n"
              << "Nine, ten, never sleep again\n";
}
