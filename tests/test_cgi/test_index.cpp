#include "../../src/core/Multiplexer.h"

int main()
{
    try
    {
        Multiplexer server("configfile/aiz.conf");
        server.run();
    }
    catch (std::exception &e)
    {
        std::cout << e.what();
    }
}
