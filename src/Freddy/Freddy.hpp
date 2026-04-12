#ifndef FREDDY_H
#define FREDDY_H

#include <iostream>

class Freddy
{
  private:
    int m_kills;

  public:
    Freddy(void);
    Freddy(const Freddy &other);
    Freddy &operator=(const Freddy &other);
    ~Freddy();

    void sing();
};

#endif
