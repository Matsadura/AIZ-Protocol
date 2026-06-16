#include <fstream>
#include "iostream"
#include "sstream"
#include <cstdlib>
#include <unistd.h>


int main() 
{
    if (unlink("tests/tempfile.txt") != 0) {
        // Handle error
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}   

