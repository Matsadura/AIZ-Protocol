#ifndef CGI_HPP
#define CGI_HPP

#include "../../src/Request/Request.hpp"
#include "../../src/Router/Router.hpp"
#include "../../src/core/Common.h"
#include <fcntl.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class CGI
{
  private:
    int    m_pipe_out[2];
    pid_t  m_pid;
    time_t m_start_time;

    std::string m_client_addr;
    std::string m_client_port;
    std::string m_server_addr;
    std::string m_server_port;

    std::vector<char *> m_envp;
    std::vector<char *> m_argv;

    void buildEnv(const Request &req, const CgiMetaData &cgiMeta);
    void buildArgv(const CgiMetaData &cgiMeta);
    void freeEnvArgv();

  public:
    CGI(const std::string &server_addr, const std::string &server_port, const std::string &client_addr,
        const std::string &client_port);
    ~CGI();

    void execute(const Request &req, const CgiMetaData &cgiMeta);

    int    getOutFd() const;
    pid_t  getPid() const;
    bool   isRunning() const;
    bool   isTimeout(time_t current_time, int seconds) const;
    void   updateStartTime();
    time_t getStartTime() const;
    bool   exitedWithFailure(int status);
    void   reapZombie(int &status);

    void waitAndClean();

    static std::string describeStatus(int status);
};

#endif
