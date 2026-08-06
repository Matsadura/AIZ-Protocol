#include "Multiplexer.h"
#include "../utils/utils.hpp"
#include "Common.h"
#include "Connections.h"
#include <csignal>

int Multiplexer::running = false;

void int_signal_handler(int n)
{
    UNUSED(n);
    std::cout << "Closing\n";
    Multiplexer::running = false;
}

Multiplexer::Multiplexer(const char *config_file) : m_epfd(-1), m_evlist(), m_config(config_file)
{
    struct epoll_event ev = {};
    m_epfd                = epoll_create(10000);
    if (m_epfd == -1)
    {
        int                err = errno;
        std::ostringstream message;
        message << "Failed to start the multiplexing system.\n";
        message << "    Cause: epoll_create() failed (" << std::strerror(err) << ").";
        throw std::runtime_error(message.str());
    }

    bool valid_server_found = false;

    std::vector<s_Server> servers = m_config.getConfig();
    for (std::size_t i = 0; i < servers.size(); i++)
    {
        for (std::map<std::string, std::vector<int> >::iterator node_it = servers[i].ports.begin();
             node_it != servers[i].ports.end(); node_it++)
        {
            const std::string &node = node_it->first;

            for (std::size_t service_index = 0; service_index < node_it->second.size(); service_index++)
            {
                const std::string &service = int_to_string(node_it->second[service_index]);

                try
                {
                    int listen_sock = m_listeners.create_new(node, service, servers[i]);
                    ev.events       = EPOLLIN;
                    ev.data.u64     = pack_data(listen_sock, LISTENER);
                    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
                    {
                        int                err = errno;
                        std::ostringstream message;
                        message << "Failed to monitor '" << node << ":" << service << "' in the multiplexer.\n";
                        message << "    Cause: epoll_ctl() failed (" << std::strerror(err) << ").";
                        m_listeners.remove(listen_sock);
                        throw std::runtime_error(message.str());
                    }
                    valid_server_found = true;
                }
                catch (std::runtime_error &e)
                {
                    LOG_ERROR("MULTIPLEXER") << e.what() << "\n";
                }
            }
        }
    }

    /**
     * SIGPIPE should be ignored! if the CGI scripts closes its reading side and we tried to write to the pipe
     * signal will kill our process
     *
     * More details:
     * When a process tries to write to a pipe for which no process has an open read descriptor, the kernel sends the
     * SIGPIPE signal to the writing process. By default, this signal kills a process
     */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, int_signal_handler);

    if (!valid_server_found)
    {
        throw std::runtime_error("No valid server found in configuration");
    }
    running = true;
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
    while (running)
    {
        std::vector<int> to_be_closed;
        ready = epoll_wait(m_epfd, m_evlist, MAX_EVENTS, EPOLL_WAIT_TIMEOUT);

        m_conns.check_for_time_out();

        if (ready > 0)
        {
            std::cout << "=========\n";
        }

        for (int j = 0; j < ready; j++)
        {
            int    fd   = unpack_conn_fd(m_evlist[j].data.u64);
            FDRole role = unpack_role(m_evlist[j].data.u64);

            log_event(m_evlist[j]);

            if (role == LISTENER)
            {
                try
                {
                    m_conns.accept_new(fd, *this);
                }
                catch (std::exception &e)
                {
                    LOG_ERROR("MULTIPLEXER") << e.what() << "\n";
                }
                continue;
            }

            Connections::connection_t *connPtr = m_conns.find(fd);
            if (connPtr == NULL)
            {
                epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
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
            else if (role == CGI_STDOUT && (m_evlist[j].events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
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
              << ((events & EPOLLIN) ? "EPOLLIN " : "") << ((events & EPOLLOUT) ? "EPOLLOUT " : "")
              << ((events & EPOLLHUP) ? "EPOLLHUP " : "") << ((events & EPOLLRDHUP) ? "EPOLLRDHUP " : "")
              << ((events & EPOLLERR) ? "EPOLLERR " : "") << "}\n";

    if (epoll_ctl(m_epfd, op, fd, &ev) == -1)
    {
        int err_code = errno;
        LOG_ERROR("MULTIPLEXER") << "Last update event failed.\n    Cause: epoll_ctl() failed ("
                                 << std::strerror(err_code) << ").\n";
        conn.closing = true;
    }
}

/**
 * React to available space in socket to send data
 */
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

        if (count <= 0)
        {
            conn.closing = true;
        }
        else
        {
            LOG_INFO("SOCKET") << "Send=" << COLOR_LIGHT_RED << count << RESET << " (id=" << conn.sockfd << ")\n";
            DebugStore::instance().append_response(conn.sockfd, data, count);
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
            DebugStore::instance().append_response(conn.sockfd, data, count);
            conn.response->consume(count);
        }
    }
}

/**
 * React to new data available in the socket
 */
void Multiplexer::sock_handle_read(Connections::connection_t &conn)
{
    if (conn.closing)
    {
        return;
    }

    char buff[BUFF_SIZE];
    long n;

    n = recv(conn.sockfd, buff, sizeof(buff), MSG_DONTWAIT);

    DebugStore::instance().append_request(conn.sockfd, buff, n);

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
}

/**
 * React to cgi process writting to stdout
 */
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
        conn.cgi.updateStartTime();
        return;
    }

    remove_interest(conn, CGI_STDOUT);

    int status = 0;
    conn.cgi.reapZombie(status);

    if (conn.cgi_response.getErrorCode() == 0)
    {
        bool failed = (count < 0) || conn.cgi.exitedWithFailure(status);
        if (failed)
        {
            conn.cgi_response.generateErrorResponse(500);
        }
        else
        {
            conn.cgi_response.appendTerminalChunk();
        }
    }
}

/**
 * Log the details of the specified event for debugging and monitoring purposes
 */
void Multiplexer::log_event(struct epoll_event ev)
{
    LOG_INFO("EPOLL") << "EVENT: " << ((ev.events & EPOLLIN) ? "EPOLLIN " : "")
                      << ((ev.events & EPOLLOUT) ? "EPOLLOUT " : "") << ((ev.events & EPOLLHUP) ? "EPOLLHUP " : "")
                      << ((ev.events & EPOLLRDHUP) ? "EPOLLRDHUP " : "") << ((ev.events & EPOLLERR) ? "EPOLLERR " : "")
                      << "(id=" << unpack_conn_fd(ev.data.u64) << ")"
                      << " type=" << get_role_string(unpack_role(ev.data.u64)) << "\n";
}

/**
 * If cgi working read its output to response and wait for it be full or to complete to start sending it
 */
void Multiplexer::react_to_cgi(Connections::connection_t &conn, uint32_t &client_events)
{
    if (!conn.cgi_active)
    {
        return;
    }

    bool cgi_res_complete = conn.cgi_response.getCgiState() == CGIResponse::CGI_COMPLETE;
    bool cgi_res_empty    = conn.cgi_response.getBodyBuffer().empty();
    bool cgi_res_full     = conn.cgi_response.isBufferFull();
    bool cgi_has_error    = conn.cgi_response.getErrorCode() != 0;
    bool cgi_already_sent = conn.cgi_response.getAlreadySendCount() != 0;

    if (cgi_has_error && !cgi_already_sent)
    {
        conn.cgi_active = false;
        remove_interest(conn, CGI_STDOUT);

        Router       router(*conn.config, conn.req.getPath(), conn.req.getMethod());
        RouterResult result = router.init_http_result(conn.cgi_response.getErrorCode());
        conn.response       = new Response(result);
        client_events |= EPOLLOUT;
    }
    else if ((cgi_res_complete && cgi_res_empty) || (cgi_has_error && cgi_already_sent))
    {
        conn.closing = true;
    }
    else if (cgi_res_full || (cgi_res_complete && !cgi_res_empty))
    {
        client_events |= EPOLLOUT;
        modify_interest(conn, CGI_STDOUT, 0);
    }
    else
    {
        modify_interest(conn, CGI_STDOUT, EPOLLIN);
    }
}

/**
 * If response calcualted wait for it to fully send then close the connection
 */
void Multiplexer::react_to_response(Connections::connection_t &conn, uint32_t &client_events)
{
    if (!conn.response)
    {
        return;
    }

    if (conn.response->isFinished())
    {
        conn.closing = true;
    }
    else
    {
        client_events |= EPOLLOUT;
    }
}

/**
 * React to request parsing errors and wait for response to finish to run the router
 */
void Multiplexer::react_to_request(Connections::connection_t &conn, uint32_t &client_events)
{
    if (conn.cgi_active || conn.response)
    {
        return;
    }

    bool req_complete = conn.req.getState() == Request::COMPLETE;
    bool req_error    = conn.req.getState() == Request::ERROR;

    if (!req_complete && !req_error)
    {
        client_events |= EPOLLIN;
    }

    if (req_error)
    {
        LOG_INFO("REQUEST") << "HTTP_Error_Code=" << conn.req.getErrorCode() << "\n";
        RouterResult result = Router::init_http_result(*conn.config, conn.req.getErrorCode());
        conn.response       = new Response(result);
        return;
    }

    if (!conn.req_routed && conn.req.isReadyForRouting())
    {
        router_request(conn);
    }

    if (conn.req.isComplete() && !conn.cgi_active && !conn.response && conn.req.getMethod() == "POST")
    {
        LOG_INFO("REQUEST") << "File: \"" << COLOR_DARK_BLUE << conn.req.getBodyFilename() << RESET << "\" Created!\n";
        conn.response = new Response(Router::init_http_result(*conn.config, 201));
    }
}

void Multiplexer::router_request(Connections::connection_t &conn)
{
    conn.req_routed = true;

    Router      router(*conn.config, conn.req.getPath(), conn.req.getMethod());
    CgiMetaData cgi_meta = router.get_cgi_metadata();

    if (cgi_meta.is_cgi)
    {
        try
        {
            conn.req.isReadyForBodyParsing();
            conn.cgi.execute(conn.req, cgi_meta);
            conn.cgi_active = true;
            add_interest(conn, CGI_STDOUT, EPOLLIN);
        }
        catch (std::runtime_error &e)
        {
            conn.cgi.waitAndClean();
            LOG_ERROR("CGI") << "[URI=" << conn.req.getURI() << "] " << e.what() << "\n";
            conn.response = new Response(Router::init_http_result(*conn.config, 500));
        }
    }
    else
    {
        RouterResult result = router.get_result();
        if (result.m_data_type == RouterResult::FILE_PATH_POST)
        {
            conn.req.isReadyForBodyParsing(result.m_data);
        }
        else
        {
            conn.req.isReadyForBodyParsing();
            conn.response = new Response(result);
        }
    }
    conn.req.appendDataAndParse("", 0); // Let the request start parsing if it has something in req buffer
}

/**
 * Change the connection state after each event
 */
void Multiplexer::update_events(Connections::connection_t &conn)
{
    uint32_t client_events = 0;

    // INFO: The calling order of this member functions matter!
    react_to_request(conn, client_events);
    react_to_cgi(conn, client_events);
    react_to_response(conn, client_events);

    modify_interest(conn, CLIENT, client_events);
}

/**
 * Get the config file associated with the listener file descriptor
 */
s_Server *Multiplexer::get_config(int fd)
{
    return m_listeners.get_listener_config(fd);
}

/**
 * Helper to get the file descritpor of a connection based on its role
 *
 * NOTE: For now connection can have two file descriptors, one associated with its socket and the cgi stdout
 */
int Multiplexer::get_role_fd(Connections::connection_t &conn, FDRole role)
{
    switch (role)
    {
        case CLIENT:
            return conn.sockfd;
        case CGI_STDOUT:
            return conn.cgi.getOutFd();
        default:
            UNREACHABLE("get_role_fd() got unknown role");
    }
}

/**
 * This returns the current value of epoll events that are monitored by multipexer
 * Used so callers can compare against the currently monitored events before calling epoll_ctl(), avoiding redundant
 * calls
 */
uint32_t &Multiplexer::get_role_events(Connections::connection_t &conn, FDRole role)
{
    switch (role)
    {
        case CLIENT:
            return conn.sock_events;
        case CGI_STDOUT:
            return conn.cgi_out_events;
        default:
            UNREACHABLE("get_role_events() got unknown role");
    }
}

/**
 * For logging..
 */
const char *Multiplexer::get_role_string(FDRole role)
{
    switch (role)
    {
        case CLIENT:
            return COLOR_DARK_PINK "CLIENT" RESET;
        case LISTENER:
            return COLOR_DARK_PINK "LISTENER" RESET;
        case CGI_STDOUT:
            return COLOR_DARK_PINK "CGI_STDOUT" RESET;
        default:
            UNREACHABLE("get_role_string() got unknown role");
    }
}

/**
 * Start monitoring a file descriptor if not already been monitored
 */
void Multiplexer::add_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
{
    uint32_t &prev_events = get_role_events(conn, role);

    if (prev_events != EPOLL_NOT_REGISTERED)
    {
        return;
    }
    prev_events = events;
    epoll_apply(conn, EPOLL_CTL_ADD, role, events);
}

/**
 * Update the monitored events of a file descriptor
 */
void Multiplexer::modify_interest(Connections::connection_t &conn, FDRole role, uint32_t events)
{
    uint32_t &prev_events = get_role_events(conn, role);

    if (prev_events == EPOLL_NOT_REGISTERED || conn.closing)
    {
        return;
    }

    if (prev_events != events)
    {
        prev_events = events;
        epoll_apply(conn, EPOLL_CTL_MOD, role, events);
    }
}

/**
 * Stop monitoring a file descriptor
 */
void Multiplexer::remove_interest(Connections::connection_t &conn, FDRole role)
{
    uint32_t &prev_events = get_role_events(conn, role);

    if (prev_events == EPOLL_NOT_REGISTERED)
    {
        return;
    }

    prev_events = EPOLL_NOT_REGISTERED;
    epoll_apply(conn, EPOLL_CTL_DEL, role, 0);
}
