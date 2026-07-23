#include "Multiplexer.h"

Multiplexer::Multiplexer(const char *config_file) : m_epfd(-1), m_evlist(), m_config(config_file)
{
    struct epoll_event ev = {};
    m_epfd                = epoll_create(10000);
    if (m_epfd == -1)
    {
        abort("epoll_create");
    }

    std::vector<s_Server> servers = m_config.getConfig();
    for (std::size_t i = 0; i < servers.size(); i++)
    {
        int listen_sock = m_listeners.create_new(servers[i]);
        ev.events       = EPOLLIN;
        ev.data.u64     = pack_data(listen_sock, LISTENER);
        if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
        {
            abort("epoll_ctl");
        }
    }

    /**
     * TODO: SIGPIPE should be ignored! if the CGI scripts closes its reading side and we tried to write to the pipe
     * signal will kill our process
     *
     * More details:
     * When a process tries to write to a pipe for which no process has an open read descriptor, the kernel sends the
     * SIGPIPE signal to the writing process. By default, this signal kills a process
     */
    signal(SIGPIPE, SIG_IGN);
}

Multiplexer::~Multiplexer()
{
    if (m_epfd != -1)
        close(m_epfd);
}

inline int Multiplexer::unpack_conn_fd(uint64_t u64)
{
    return static_cast<int>(u64 >> 32);
}

Multiplexer::FDRole Multiplexer::unpack_role(uint64_t u64)
{
    return static_cast<FDRole>(u64 & 0xFFFFFFFF);
}

inline uint64_t Multiplexer::pack_data(int conn_fd, FDRole role)
{
    return (static_cast<uint64_t>(conn_fd) << 32) | static_cast<uint64_t>(role);
}

/**
 * Run the event loop to monitor and handle events on the listener sockets and active connections
 */
void Multiplexer::run()
{
    int ready;
    while (true)
    {
        ready = epoll_wait(m_epfd, m_evlist, MAX_EVENTS, -1);
        std::vector<int> to_be_closed;
        std::cout << "=========\n";
        for (int j = 0; j < ready; j++)
        {
            int    fd   = unpack_conn_fd(m_evlist[j].data.u64);
            FDRole role = unpack_role(m_evlist[j].data.u64);

            log_event(m_evlist[j]);
            if (role == LISTENER)
            {
                m_conns.accept_new(fd, *this);
                continue;
            }

            Connections::connection_t *connPtr = m_conns.find(fd);
            if (connPtr == NULL)
            {
                continue;
            }

            Connections::connection_t &conn = *connPtr;

            if (role == CLIENT)
            {
                if (m_evlist[j].events & EPOLLIN)
                {
                    sock_handle_read(conn);
                }

                if (m_evlist[j].events & EPOLLOUT)
                {
                    sock_handle_write(conn);
                }
            }
            if (role == CGI_STDOUT && (m_evlist[j].events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
            {
                cgi_handle_out(conn);
            }
            update_events(conn);
            if (conn.closing)
            {
                to_be_closed.push_back(fd);
            }
        }
        for (std::size_t i = 0; i < to_be_closed.size(); i++)
        {
            m_conns.close_connection(to_be_closed[i], *this);
        }
    }
}

void Multiplexer::epoll_apply(Connections::connection_t &conn, int op, FDRole role, uint32_t events)
{
    struct epoll_event ev = {};
    ev.events             = events;
    ev.data.u64           = pack_data(conn.sockfd, role);
    int fd                = get_role_fd(conn, role);

    LOG_INFO("EPOLL");
    switch (op)
    {
        case EPOLL_CTL_ADD:
            std::cout << "ADD";
            break;
        case EPOLL_CTL_MOD:
            std::cout << "MOD";
            break;
        case EPOLL_CTL_DEL:
            std::cout << "DEL";
            break;
        default:
            UNREACHABLE("Wrong value for op argument\n");
    }

    std::cout << " fd=" << fd << " type=" << get_role_string(role) << " events={"
              << ((events & EPOLLIN) ? "EPOLLIN |" : "") << ((events & EPOLLOUT) ? "EPOLLOUT |" : "")
              << ((events & EPOLLHUP) ? "EPOLLHUP |" : "") << ((events & EPOLLRDHUP) ? "EPOLLRDHUP |" : "")
              << ((events & EPOLLERR) ? "EPOLLERR |" : "") << "}\n";

    if (epoll_ctl(m_epfd, op, fd, &ev) == -1)
    {
        abort("epoll_ctl");
    }
}

void Multiplexer::sock_handle_write(Connections::connection_t &conn)
{
    if (conn.closing)
    {
        return;
    }

    if (conn.cgi_active)
    {
        const char *data = conn.cgi_response.getBodyBuffer().data();
        size_t      size = conn.cgi_response.getBodyBuffer().size();

        long count = write(conn.sockfd, data, size);

        LOG_INFO("SOCKET") << "Send=" << COLOR_LIGHT_RED << count << RESET << " (id=" << conn.sockfd << ")\n";

        if (count <= 0)
        {
            conn.closing = true;
        }
        else
        {
            conn.cgi_response.consumeBodyChunk(count);
        }
    }
    else if (conn.response)
    {
        const char *data = conn.response->getResponseBuffer().data();
        size_t      size = conn.response->getResponseBuffer().size();

        long count = write(conn.sockfd, data, size);

        LOG_INFO("SOCKET") << "Send=" << COLOR_LIGHT_RED << count << RESET << " (id=" << conn.sockfd << ")\n";

        if (count <= 0)
        {
            conn.closing = true;
        }
        else
        {
            conn.response->consume(count);
        }
    }
}

void Multiplexer::sock_handle_read(Connections::connection_t &conn)
{
    if (conn.closing)
    {
        return;
    }

    char buff[BUFF_SIZE];
    long n;

    n = recv(conn.sockfd, buff, sizeof(buff), MSG_DONTWAIT);

    if (n <= 0)
    {
        LOG_INFO("CONNECTIONS") << "Read peer shutdown (fd=" << conn.sockfd << ")\n";
        conn.closing = true;
        return;
    }

    // std::printf("--------------------------------------------------------------\n");
    // std::printf(" CLIENT SENED: %.*s", (int)n, buff);
    // std::printf("--------------------------------------------------------------\n");

    conn.req.appendDataAndParse(buff, n);

    LOG_INFO("SOCKET") << "RECIEVED=" << COLOR_LIGHT_RED << n << RESET " (id=" << conn.sockfd << ")\n";

    if (conn.req.getState() == Request::ERROR)
    {
        LOG_INFO("REQUEST") << "HTTP_Error_Code=" << conn.req.getErrorCode() << "\n";
    }
    if (conn.req.isReadyForRouting() && !conn.cgi_active && !conn.response)
    {
        CgiMetaData cgi_meta = is_cgi_request(*conn.config, conn.req);

        if (cgi_meta.is_cgi)
        {
            std::cout << "CGI script: " << cgi_meta.script_path << "\n";
            std::cout << "CGI path info: " << cgi_meta.path_info << "\n";
            std::cout << "CGI interpreter path: " << cgi_meta.interpreter_path << "\n";
            conn.cgi.execute(conn.req, cgi_meta);
            conn.cgi_active = true;
            add_interest(conn, CGI_STDOUT, 0);
            conn.req.isReadyForBodyParsing();
            conn.req.appendDataAndParse(buff, 0);
        }
        else
        {
            Router       router(*conn.config, conn.req.getPath(), conn.req.getMethod());
            RouterResult result = router.get_result();
            conn.response       = new Response(result); // TODO: Make sure you always delete this
        }
    }
}

void Multiplexer::cgi_handle_out(Connections::connection_t &conn)
{
    if (conn.closing)
    {
        return;
    }

    char buff[BUFF_SIZE];

    ssize_t count = read(conn.cgi.getOutFd(), buff, BUFF_SIZE);

    if (count > 0)
    {
        LOG_INFO("CGI") << "STDOUT_SEND=" << COLOR_LIGHT_RED << count << RESET << " (id=" << conn.sockfd << ")\n";
        conn.cgi_response.appendCgiData(buff, count);
        return;
    }

    remove_interest(conn, CGI_STDOUT);

    int  status = 0;
    bool reaped = conn.cgi.reapIfExited(status);
    bool failed = (count < 0) || (reaped && conn.cgi.exitedWithFailure(status));

    if (failed)
    {
        conn.cgi_response.generateErrorResponse(500);
    }
    else
    {
        conn.cgi_response.appendTerminalChunk();
    }
}

/**
 * Log the details of the specified event for debugging and monitoring purposes
 */
void Multiplexer::log_event(struct epoll_event ev)
{
    LOG_INFO("EPOLL") << "event: " << ((ev.events & EPOLLIN) ? "EPOLLIN " : "")
                      << ((ev.events & EPOLLOUT) ? "EPOLLOUT " : "") << ((ev.events & EPOLLHUP) ? "EPOLLHUP " : "")
                      << ((ev.events & EPOLLRDHUP) ? "EPOLLRDHUP " : "") << ((ev.events & EPOLLERR) ? "EPOLLERR " : "")
                      << "(id=" << unpack_conn_fd(ev.data.u64) << ")"
                      << " type=" << get_role_string(unpack_role(ev.data.u64)) << "\n";
}

void Multiplexer::update_events(Connections::connection_t &conn)
{
    uint32_t client_events = 0;

    bool req_complete = conn.req.getState() == Request::COMPLETE;

    if (req_complete)
    {
        client_events |= EPOLLOUT;
    }
    else
    {
        client_events |= EPOLLIN;
    }

    if (conn.cgi_active)
    {
        bool cgi_res_complete = conn.cgi_response.getCgiState() == CGIResponse::CGI_COMPLETE;
        bool cgi_res_empty    = conn.cgi_response.getBodyBuffer().empty();
        bool cgi_res_full     = conn.cgi_response.isBufferFull();

        if (cgi_res_complete && cgi_res_empty)
        {
            remove_interest(conn, CGI_STDOUT);
            conn.closing = true; // TODO: This will close the connection immediately should we persist the connection?
                                 // or should we act according the keep-alive header?
        }
        else if (cgi_res_full || (cgi_res_complete && !cgi_res_empty))
        {
            client_events |= EPOLLOUT;
            modify_interest(conn, CGI_STDOUT, 0);
        }
    }
    else if (conn.response && conn.response->isFinished())
    {
        client_events = 0;
    }
    modify_interest(conn, CLIENT, client_events);
}
