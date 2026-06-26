#ifndef CGI_HPP
#define CGI_HPP

#include "../../src/Request/Request.hpp"
#include <string>
#include <unistd.h>
#include <vector>

class CGI
{
  private:
    int   m_pipe_in[2];
    int   m_pipe_out[2];
    pid_t m_pid;

    std::vector<char *> m_envp;
    std::vector<char *> m_argv;

    void buildEnv(const Request &req);
    void buildArgv(const std::string &scriptPath);

  public:
    CGI();
    ~CGI();

    void execute(const Request &req, const std::string &scriptPath);

    int getReadFd() const;
    int getWriteFd() const;

    void waitAndClean();
};

#endif