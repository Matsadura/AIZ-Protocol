#include "../../src/Request/Request.hpp"
#include "../../src/Response/Response.hpp"
#include "../../src/config_file_parser/parser/configfile.hpp"

#define HTML_ROOT "/tmp/webserv_test/www/html/"

int main()
{

    Response rsp(RouterResult(200, HTML_ROOT "about.html", RouterResult::STRING_BUFFER));

    std::ofstream out("result.bin", std::ios::binary);

    while (!rsp.isFinished())
    {
        const std::vector<char> &buff = rsp.getResponseBuffer();

        std::cout << "[";
        for (size_t i = 0; i < buff.size(); i++)
            std::cout << buff[i];
        std::cout << "]\n=====================\n";

        size_t sent = std::rand() % (buff.size() + 1);

        out.write(buff.data(), sent);
        std::cout << "sent = " << sent << "\n";
        rsp.consume(sent);
    }
    // std::vector<char>buff = rsp.getResponseBuffer();
    // for(size_t i = 0; i < buff.size() ; i++)
    //     std::cout << buff[i];
    // std::cout << "\n";
    // rsp.consume(10000);
    // if(!rsp.isFinished())
    //     std::cout << "Not finished\n";
    // buff = rsp.getResponseBuffer();
    // std::cout << "[";
    // for(size_t i = 0; i < buff.size() ; i++)
    //     std::cout << buff[i];
    // std::cout << "]";
    // std::cout << "\n";
}