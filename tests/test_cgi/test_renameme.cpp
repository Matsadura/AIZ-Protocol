#include "../../src/core/Multiplexer.h"

int main()
{
    Multiplexer server("configfile/router.conf");
    server.run();
}
