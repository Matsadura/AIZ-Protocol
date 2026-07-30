#include "../../src/core/Multiplexer.h"

int main()
{
    Multiplexer server("configfile/aiz.conf");
    server.run();
}
