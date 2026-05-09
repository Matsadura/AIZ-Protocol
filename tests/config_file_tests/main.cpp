#include "../parser/parser.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./lexer <config_file>\n";
        return 1;
    }

    try
    {
        std::string file_name = argv[1];
        lexer lx(file_name);
        std::vector<s_token> tokens = lx.tokenize();
        Parser parser(tokens); // throws
        parser.print_directives();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}