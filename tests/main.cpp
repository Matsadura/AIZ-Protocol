#include "../src/core/Multiplexer.h"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "Usage: " << av[0] << " <file.conf>\n";
        return 1;
    }

    try
    {
        Multiplexer server(av[1]);
        server.run();
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: " << e.what() << "\n";
    }
}
